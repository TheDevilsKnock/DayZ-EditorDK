class EditorNode: Managed
{
	protected ref EditorNodeView m_EditorNodeView;
	
	ref array<ref EditorNode> Children = {};
	protected EditorNode m_Parent;
	
	void InsertChild(notnull EditorNode node)
	{
		Children.Insert(node);
		node.m_Parent = this;
	}

	bool HasView()
	{
		return m_EditorNodeView != null;
	}
		
	EditorNodeView GetView()
	{
		return m_EditorNodeView;
	}
	
	EditorNode GetParent()
	{
		return m_Parent;
	}

	EditorNodeView EnsureView()
	{
		return CreateView();
	}

	void EnsureChildViewsAttached()
	{
		if (m_EditorNodeView) {
			m_EditorNodeView.EnsureChildViewsAttached();
		}
	}

	void ReleaseViewRecursive()
	{
		foreach (EditorNode child: Children) {
			child.ReleaseViewRecursive();
		}

		if (m_EditorNodeView) {
			delete m_EditorNodeView;
			m_EditorNodeView = null;
		}
	}
	
	EditorNodeView CreateView()
	{
		if (!m_EditorNodeView) {
			m_EditorNodeView = new EditorNodeView(this);
		}
		
		return m_EditorNodeView;
	}
	
	bool FilterType(string filter, bool favorites)
	{
		return false;
	}
}

class EditorFolderNode: EditorNode
{
	protected string m_Text;
	
	void EditorFolderNode(string text)
	{
		m_Text = text;
	}
	
	string GetText()
	{
		return m_Text;
	}
	
	override EditorNodeView CreateView()
	{
		if (!m_EditorNodeView) {
			m_EditorNodeView = new EditorFolderListNode(this);
		}
		
		return m_EditorNodeView;
	}
	
	override bool FilterType(string filter, bool favorites)
	{
		// When a filter or favorites toggle is active, show this folder only if a descendant matches.
		// allows the pre pass in RefreshSearchBar to detect which folders to expand.
		if (favorites || filter.Length() > 0) {
			foreach (EditorNode child: Children) {
				if (child.FilterType(filter, favorites)) {
					return true;
				}
			}
			return false;
		}
		// No filter active, always visible.
		return true;
	}
}

class EditorNodeView: ScriptView
{	
	static EditorNodeView s_SelectedNode;
	static ref EditorNodeViewContextMenu s_ContextMenu;
			
	protected bool m_IsCollapsed = true;
	
	protected EditorNode m_Node;
	protected int m_AttachedChildCount;
	
	protected bool m_IsFilterExpanded;
	
	Widget Collapse, IconFrame, Hide, Panel, ChildrenHeight, Favorite;
	ButtonWidget CollapseButton, HideButton, FavoriteButton;
	ImageWidget CollapseIcon, IconImage, HideIcon, FavoriteIcon;
	TextWidget Text;
	EditBoxWidget Edit;
	WrapSpacerWidget Children;
	
	void EditorNodeView(notnull EditorNode node)
	{
		m_Node = node;
		Collapse.Show(false);
		m_LayoutRoot.SetSort(1);
		CollapseIcon.LoadImageFile(1, "set:solid image:square_minus");
		CollapseIcon.LoadImageFile(0, "set:regular image:square_plus");
	}
	
	void ~EditorNodeView()
	{
	}
		
	void InsertChild(notnull EditorNodeView list_node)
	{
		Collapse.Show(true);
		Children.AddChild(list_node.GetLayoutRoot());
	}

	void EnsureChildViewsAttached()
	{
		Collapse.Show(m_Node.Children.Count() > 0);
		if (m_AttachedChildCount >= m_Node.Children.Count()) {
			return;
		}

		for (int i = m_AttachedChildCount; i < m_Node.Children.Count(); i++) {
			EditorNodeView child_view = m_Node.Children[i].EnsureView();
			if (child_view) {
				InsertChild(child_view);
			}
		}

		m_AttachedChildCount = m_Node.Children.Count();
	}

	void SetChildrenVisible(bool state)
	{
		Collapse.Show(m_Node.Children.Count() > 0);
		Children.Show(state);
		CollapseIcon.SetImage(state);

		float child_width, child_height;
		Children.Update();
		Children.GetScreenSize(child_width, child_height);
		child_height *= state;
		ChildrenHeight.SetScreenSize(2, child_height);

		int screen_x, screen_y;
		GetScreenSize(screen_x, screen_y);
		m_LayoutRoot.SetScreenSize(screen_x, child_height + 24 * screen_y / 1080.0);
	}
	
	void SetCollapsed(bool collapsed)
	{			
		if (!m_Node.Children.Count()) {
			return;
		}
		
		if (!collapsed) {
			m_Node.EnsureChildViewsAttached();
		}
		
		m_IsCollapsed = collapsed;
		m_IsFilterExpanded = false; // user change always clears filter expansion state
		SetChildrenVisible(!collapsed);
		
		// If you are setting this as collapsed, the parents must be uncollapsed because you can access it. we are using this to update the collapse state
		if (m_Node.GetParent() && m_Node.GetParent().HasView()) {
			m_Node.GetParent().GetView().SetCollapsed(false);
		}
	}
			
	// Applies or clears filter expansion without changing m_IsCollapsed.
	// Used by RefreshSearchBar to show matching descendants while preserving user collapse state.
	void SetFilterExpanded(bool state)
	{
		if (!m_Node.Children.Count()) {
			return;
		}

		if (state) {
			m_Node.EnsureChildViewsAttached();
		}

		m_IsFilterExpanded = state;
		SetChildrenVisible(!m_IsCollapsed || m_IsFilterExpanded);
	}
			
	void CollapseAll()
	{		
		for (int i = 0; i < m_Node.Children.Count(); i++) {
			if (m_Node.Children[i].HasView()) {
				m_Node.Children[i].GetView().CollapseAll();
			}
		}
		
		SetCollapsed(true);
	}
	
	void ExpandAll()
	{		
		m_Node.EnsureChildViewsAttached();

		for (int i = 0; i < m_Node.Children.Count(); i++) {
			m_Node.Children[i].EnsureView().ExpandAll();
		}
		
		SetCollapsed(false);
	}
					
	bool IsCollapsed()
	{
		return m_IsCollapsed;
	}

	bool IsFilterExpanded()
	{
		return m_IsFilterExpanded;
	}
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		switch (w) {
			case CollapseButton: {
				if (button == MouseState.LEFT) {
					SetCollapsed(!IsCollapsed());	
				}	
							
				break;
			}
		}
						
		return true;
	}		
	
	override bool OnDrag(Widget w, int x, int y)
	{
		w.SetPos(0, 0);
		
		return super.OnDrag(w, x, y);
	}
	
	override bool OnDragging(Widget w, int x, int y, Widget reciever)
	{
		w.SetPos(0, 0);
		
		return super.OnDragging(w, x, y, reciever);
	}
	
	override bool OnDrop(Widget w, int x, int y, Widget reciever)
	{
		w.SetPos(0, 0);
		
		return super.OnDrop(w, x, y, reciever);
	}	
	
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{		
		return true;
	}
	
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{				
		if (s_SelectedNode) {
			s_SelectedNode.Panel.SetColor(0);
		}
		
		s_SelectedNode = this;		
		Panel.SetColor(g_Editor.GetSettings().SelectionColor);
				
		SetFocus(null);

		return true;
	}
	
	protected bool IsSelected()
	{
		return s_SelectedNode == this;
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		switch (w) {
			case ChildrenHeight: {
				WidgetAnimator.CancelAnimate(ChildrenHeight);
				ChildrenHeight.SetColor(GetEditor().GetSettings().HighlightColor);
				break;
			}
			
			case m_LayoutRoot: {
				if (!IsSelected()) {
					WidgetAnimator.CancelAnimate(Panel);
					Panel.SetColor(GetEditor().GetSettings().HighlightColor);
				}
				
				break;
			}
		}
		
		return true;
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		switch (w) {
			case ChildrenHeight: {
				WidgetAnimator.AnimateColor(ChildrenHeight, 0xffc8c8c8, 100);
				break;
			}
			
			case m_LayoutRoot: {
				if (!IsSelected()) {
					WidgetAnimator.Animate(Panel, WidgetAnimatorProperty.COLOR_A, 0, 100);
				}
				break;
			}
		}
		
		return true;
	}
	
	override string GetLayoutFile()
	{
		return "DayZEditor\\GUI\\layouts\\items\\NodeTreeView.layout";
	}
			
	protected override bool UseUpdateLoop()
	{
		return false;
	}
				
	// All of these slow down the process
	override bool OnUpdate(Widget w)
	{
		return true;
	}
	
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		return true;
	}
	
	override bool OnResize(Widget w, int x, int y)
	{
		return true;
	}
}

class EditorFolderListNode: EditorNodeView
{
	protected string m_Text;
	
	void EditorFolderListNode(notnull EditorNode node)
	{
		EditorFolderNode folder_node = EditorFolderNode.Cast(node);
		if (folder_node) {
			m_Text = folder_node.GetText();
			Text.SetText(m_Text);
		}
		
		m_Text.ToLower();
		m_LayoutRoot.SetSort(0);
		m_LayoutRoot.ClearFlags(WidgetFlags.DRAGGABLE);
	}
	
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		super.OnDoubleClick(w, x, y, button);
		
		if (w == m_LayoutRoot) {
			SetCollapsed(!IsCollapsed());	
		}
		
		return true;
	}
	
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button == MouseState.RIGHT) {
			s_ContextMenu = new EditorNodeViewContextMenu(x, y, this);
		}
		
		GetEditor().ClearHand();
		
		return super.OnMouseButtonDown(w, x, y, button);
	}
}