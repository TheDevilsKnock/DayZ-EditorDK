[RegisterEditorCommand(EditorLinearArrayCommand)]
class EditorLinearArrayCommand: EditorCommand
{
    // States
    protected bool m_IsActive;
    protected vector m_StartPos;
    protected vector m_EndPos;
    protected bool m_CachedMagnetMode;
    
    // Configs
    float m_Margin = 0.0;           
    float m_RotationOffset = 90.0;
    bool m_StretchToFit = false;
    bool m_AlignPitchToSlope = true; 
    bool m_FollowTerrain = false;
    
    // Cache
    protected float m_ObjectLengthCache = -1;
    protected int m_ActivePreviewCount = 0;
    
    // References
    protected ref array<Object> m_PreviewObjects = {};
    protected EditorPlaceableItem m_SourceItem;
    protected EditorObject m_AnchorObject; 

    void ~EditorLinearArrayCommand()
    {
        Shutdown();
    }

    // EditorCommand Overrides
    override string GetName() { return "Linear Array Tool"; }
    override Symbols GetSymbol() { return Symbols.ARROWS_LEFT_RIGHT; }
    override ShortcutKeys GetShortcut() { return { KeyCode.KC_LCONTROL, KeyCode.KC_B }; }
    override LinearColor GetColor() { return LinearColor.CYAN; }

    override bool IsToggled() { return m_IsActive; }
    
    override bool CanExecute()
    {
        return GetEditor().GetSelectedObjects().Count() > 0;
    }

    protected override bool Execute(Class sender, CommandArgs args)
    {
        super.Execute(sender, args);
        
        m_IsActive = !m_IsActive;
        
        if (m_IsActive) {
            InitializeFromSelection();
            GetGame().GetUpdateQueue(CALL_CATEGORY_GAMEPLAY).Insert(OnUpdate);
            
            // Generate the toolbar in the sidebar
            GenerateUI();
            
            GetEditor().GetEditorHud().CreateNotification("Linear Array: ENABLED.");
        } 
        else {
            Shutdown();
            GetEditor().GetEditorHud().CreateNotification("Linear Array: DISABLED");
        }

        return true;
    }

    // UI Logic

    protected void GenerateUI()
    {
        EditorHud hud = GetEditor().GetEditorHud();
        EditorHudController hudController = hud.GetTemplateController();
        if (!hudController) return;

        // Clear & Populate
        hudController.ActiveToolProperties.Clear();
        
        // Sliders/CheckBoxes
        hudController.ActiveToolProperties.Insert(new SliderPrefab("Gap / Margin", this, "m_Margin", -2.0, 10.0, 0.05));
        hudController.ActiveToolProperties.Insert(new SliderPrefab("Base Rotation", this, "m_RotationOffset", 0, 360, 45));
        hudController.ActiveToolProperties.Insert(new CheckBoxPrefab("Stretch to Fit", this, "m_StretchToFit"));
        hudController.ActiveToolProperties.Insert(new CheckBoxPrefab("Follow Terrain", this, "m_FollowTerrain"));
        hudController.ActiveToolProperties.Insert(new CheckBoxPrefab("Align Pitch", this, "m_AlignPitchToSlope"));

        // Show container
        if (hud.ActiveToolProperties) {
            hud.ActiveToolProperties.Show(true);
        }
        
        // Force layout refresh (Delayed to allow children to spawn and size to calculate)
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(hud.RefreshLayout, 20, false); 
    }

    protected void ClearUI()
    {
        EditorHud hud = GetEditor().GetEditorHud();
        EditorHudController hudController = hud.GetTemplateController();
        
        if (hudController) {
            hudController.ActiveToolProperties.Clear();
        }

        // Hide the container
        if (hud.ActiveToolProperties) {
            hud.ActiveToolProperties.Show(false);
        }

        // Refresh layout to resize scrollbar
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(hud.RefreshLayout, 1, false);
    }

    void PropertyChanged(string property_name)
    {
        if (property_name == "m_FollowTerrain") {
            GetEditor().MagnetMode = m_FollowTerrain;
            m_CachedMagnetMode = m_FollowTerrain;
        }

        UpdatePreviews();
    }

    protected void InitializeFromSelection()
    {
        EditorObjectMap selection = GetEditor().GetSelectedObjects();
        if (selection.Count() > 0) {
            m_AnchorObject = selection.GetElement(0);
            m_StartPos = m_AnchorObject.GetPosition();
            m_EndPos = m_StartPos; 
            
            string type = m_AnchorObject.GetType();
            m_SourceItem = GetEditor().GetObjectManager().GetPlaceableObject(type);
            
            if (!m_SourceItem) {
                m_SourceItem = EditorPlaceableItem.Create(type, type, 2); 
            }

            m_ObjectLengthCache = -1;
            
            // Sync local with global state
            m_CachedMagnetMode = GetEditor().MagnetMode;
            m_FollowTerrain = m_CachedMagnetMode; 
            
        } else {
            Shutdown();
        }
    }

    protected void Shutdown()
    {
        m_IsActive = false;
        
        // Clean up previews
        foreach(Object obj : m_PreviewObjects) {
            if (obj) GetGame().ObjectDelete(obj);
        }
        m_PreviewObjects.Clear();
        m_ActivePreviewCount = 0;
        
        // Clean up UI
        ClearUI();
        
        GetGame().GetUpdateQueue(CALL_CATEGORY_GAMEPLAY).Remove(OnUpdate);
        m_AnchorObject = null;
    }

    // Quick adjustments
    void OnMouseWheel(int wheel)
    {
        if (!m_IsActive) return;

        if (GetEditor().IsShiftDown()) {
            m_RotationOffset += (wheel * 90);
        } else {
            m_Margin += (wheel * 0.1); 
        }
        
        UpdatePreviews();
    }

    void OnUpdate(float dt)
    {
        if (!m_IsActive || !m_AnchorObject) {
            Shutdown();
            return;
        }
        
        // Handle Global Magnet Mode Toggle via Hotkey (CTRL+1)
        if (m_CachedMagnetMode != GetEditor().MagnetMode) {
            m_CachedMagnetMode = GetEditor().MagnetMode;
            m_FollowTerrain = m_CachedMagnetMode;
            // TODO: Refresh UI checkbox state here
            UpdatePreviews();
        }

        Input input = GetGame().GetInput();
        
        if (input.LocalPress("UAFire", false)) {
            if (!GetWidgetUnderCursor()) {
                CommitObjects();
            }
        }
        
        UpdateMousePositions();
        UpdatePreviews();
    }

    protected void CommitObjects()
    {
        if (m_ActivePreviewCount == 0) return;
        if (!m_SourceItem) return;
        
        EditorAction batchAction = new EditorAction("Delete", "Create");
        EditorObject lastCreatedObj = null;
        
        // Only iterate up to active count
        for (int i = 0; i < m_ActivePreviewCount; i++) {
            Object preview = m_PreviewObjects[i];
            if (!preview) continue;
            
            vector transform[4];
            preview.GetTransform(transform);
            
            vector pos = transform[3];
            vector ori = Math3D.MatrixToAngles(transform);
            float scale = 1.0; 
            
            EditorObjectData data = EditorObjectData.Create(m_SourceItem.Type, pos, ori, scale, EFE_DEFAULT);
            EditorObject obj = GetEditor().CreateObject(data, false); 
            
            if (obj) {
                // Apply stretch scale if necessary
                if (m_StretchToFit) {
                     // Re-apply matrix which contains scaling
                    obj.SetTransform(transform);
                    obj.Update();
                }
                
                batchAction.InsertUndoParameter(new Param1<int>(obj.GetID()));
                batchAction.InsertRedoParameter(new Param1<EditorObjectData>(data));
                lastCreatedObj = obj;
            }
        }
        
        GetEditor().InsertAction(batchAction);
        
        if (lastCreatedObj) {
            GetEditor().ClearSelection();
            GetEditor().SelectObject(lastCreatedObj);
            InitializeFromSelection(); 
        }
    }

    protected void UpdateMousePositions()
    {
        if (m_AnchorObject) {
            m_StartPos = m_AnchorObject.GetPosition();
        }
        
        Raycast rc = GetEditor().GetCursorRaycastModeSafeEx(null, true);
        if (rc) {
            m_EndPos = rc.Bounce.Position;
        } else {
            m_EndPos = GetEditor().GetCursorRay().GetPoint(5.0);
        }
    }

    float GetObjectLength(Object obj)
    {
        if (m_ObjectLengthCache > 0) return m_ObjectLengthCache;
        if (!obj) return 1.0;

        vector minMax[2];
        obj.ClippingInfo(minMax);
        float len = Math.AbsFloat(minMax[1][0] - minMax[0][0]);
        
        if (len < 0.1) len = 1.0; 
        
        m_ObjectLengthCache = len;
        return len;
    }

    // Pooling Logic
    protected void AdjustPoolSize(int count)
    {
        if (!m_SourceItem) return;

        // Grow pool if needed
        while (m_PreviewObjects.Count() < count) {
            Object obj = GetGame().CreateObjectEx(m_SourceItem.Type, vector.Zero, ECE_LOCAL);
            if (obj) {
                dBodyDestroy(obj); 
                obj.SetFlags(EntityFlags.STATIC, false); 
                m_PreviewObjects.Insert(obj);
            } else {
                break;
            }
        }

        m_ActivePreviewCount = count;

        // Hide unused objects
        for (int i = count; i < m_PreviewObjects.Count(); i++) {
            if (m_PreviewObjects[i]) {
                m_PreviewObjects[i].SetPosition("0 -5000 0");
            }
        }
    }

    protected void UpdatePreviews()
    {
        if (!m_AnchorObject || !m_SourceItem) return;

        vector dirVector = m_EndPos - m_StartPos;
        float totalDistance = dirVector.Length();

        if (totalDistance < 0.01) {
            AdjustPoolSize(0);
            return;
        }

        dirVector.Normalize();

        if (m_PreviewObjects.Count() == 0) AdjustPoolSize(1);
        
        Object sampleObject;
        if (m_PreviewObjects.Count() > 0) {
            sampleObject = m_PreviewObjects[0];
        } else {
            if (m_AnchorObject) {
                sampleObject = m_AnchorObject.GetWorldObject();
            }
        }
        
        float objectLength = GetObjectLength(sampleObject);
        
        float heightOffset = 0.0;
        if (sampleObject) {
            vector boundingCenter = sampleObject.GetBoundingCenter();
            heightOffset = boundingCenter[1];
        }
        
        float effectiveLength = objectLength + m_Margin;
        if (effectiveLength <= 0.1) effectiveLength = 0.1;

        int count = Math.Floor((totalDistance + 0.01) / effectiveLength);
        if (count > 100) count = 100;
        
        AdjustPoolSize(count);

        float stretchFactor = 1.0;
        if (m_StretchToFit && count > 0) {
            float targetTotalLength = count * effectiveLength;
            if (targetTotalLength > 0.01) {
                stretchFactor = totalDistance / targetTotalLength;
            }
        }
        
        float currentStepLength = effectiveLength * stretchFactor;

        vector rotationMatrix[3];
        Math3D.YawPitchRollMatrix(Vector(m_RotationOffset, 0, 0), rotationMatrix);

        for (int i = 0; i < count; i++)
        {
            Object preview = m_PreviewObjects[i];
            if (!preview) continue;
            
            int stepIndex = i + 1; 

            vector pos = m_StartPos + (dirVector * (stepIndex * currentStepLength));
            vector nextPos = m_StartPos + (dirVector * ((stepIndex + 1) * currentStepLength));

            if (GetEditor().MagnetMode) {
                pos[1] = GetGame().SurfaceY(pos[0], pos[2]) + heightOffset;
                nextPos[1] = GetGame().SurfaceY(nextPos[0], nextPos[2]) + heightOffset;
            } else {
                float t1 = (float)stepIndex / (float)(count + 1);
                pos[1] = Math.Lerp(m_StartPos[1], m_EndPos[1], t1);
                
                float tNext = (float)(stepIndex + 1) / (float)(count + 1);
                nextPos[1] = Math.Lerp(m_StartPos[1], m_EndPos[1], tNext);
            }

            vector segmentDir = (nextPos - pos).Normalized();
            
            vector mat[4];
            mat[2] = segmentDir; 
            
            vector worldUp = "0 1 0";
            if (Math.AbsFloat(vector.Dot(mat[2], worldUp)) > 0.99) {
                worldUp = "0 0 1"; 
            }

            vector right = worldUp * mat[2]; 
            right.Normalize();
            mat[0] = right;
            
            vector up = mat[2] * mat[0]; 
            mat[1] = up;
            mat[3] = pos;

            Math3D.MatrixMultiply3(mat, rotationMatrix, mat);
            
            if (!m_AlignPitchToSlope) {
                vector calculatedAngles = Math3D.MatrixToAngles(mat);
                float pitch = calculatedAngles[1];
                calculatedAngles[1] = 0;
                calculatedAngles[2] = pitch;
                Math3D.YawPitchRollMatrix(calculatedAngles, mat);
                mat[3] = pos;
            }

            if (m_StretchToFit && stretchFactor != 1.0) {
                vector scaleMat[3];
                Math3D.MatrixIdentity3(scaleMat);
                scaleMat[0][0] = stretchFactor; 
                Math3D.MatrixMultiply3(mat, scaleMat, mat);
            } 
            
            preview.SetTransform(mat);
        }
    }

}