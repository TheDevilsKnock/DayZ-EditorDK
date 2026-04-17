enum EditorBuildMenuFilterButtonType
{
	EDITOR_BUILD_MENU_FILTER_SOURCE = 0,
	EDITOR_BUILD_MENU_FILTER_TAB = 1,
	EDITOR_BUILD_MENU_FILTER_SUBCATEGORY = 2
}

class EditorBuildMenuFilterButton: ScriptView
{
	static const float MIN_WIDTH = 82;
	static const float HEIGHT = 26;
	static const float HORIZONTAL_PADDING = 24;

	protected EditorBuildMenuView m_Owner;
	protected string m_Id;
	protected int m_Value = -1;
	protected int m_ButtonType;
	protected bool m_IsSelected;
	protected bool m_IsHovered;

	Widget BuildMenuFilterButtonRoot;
	ButtonWidget BuildMenuFilterButton;
	TextWidget BuildMenuFilterButtonLabel;

	void EditorBuildMenuFilterButton(EditorBuildMenuView owner, string label, int button_type, string id = string.Empty, int value = -1)
	{
		m_Owner = owner;
		m_Id = id;
		m_Value = value;
		m_ButtonType = button_type;

		BuildMenuFilterButtonLabel.SetText(label);
		ResizeToContent();
	}

	void SetSelected(bool selected)
	{
		m_IsSelected = selected;
		UpdateVisualState();
	}

	protected void UpdateVisualState()
	{
		if (m_IsSelected) {
			BuildMenuFilterButtonRoot.SetColor(GetEditor().GetSettings().SelectionColor);
			BuildMenuFilterButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		} else if (m_IsHovered) {
			BuildMenuFilterButtonRoot.SetColor(GetEditor().GetSettings().HighlightColor);
			BuildMenuFilterButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		} else {
			BuildMenuFilterButtonRoot.SetColor(ARGB(230, 43, 46, 52));
			BuildMenuFilterButtonLabel.SetColor(ARGB(235, 201, 208, 216));
		}
	}

	string GetId()
	{
		return m_Id;
	}

	int GetValue()
	{
		return m_Value;
	}

	int GetButtonType()
	{
		return m_ButtonType;
	}

	void ResizeToContent()
	{
		int label_width, label_height;
		BuildMenuFilterButtonLabel.GetTextSize(label_width, label_height);

		float button_width = label_width + HORIZONTAL_PADDING;
		if (button_width < MIN_WIDTH) {
			button_width = MIN_WIDTH;
		}

		BuildMenuFilterButtonRoot.SetSize(button_width, HEIGHT);
		BuildMenuFilterButton.SetSize(button_width, HEIGHT);
	}

	float GetWidth()
	{
		float width, height;
		BuildMenuFilterButtonRoot.GetSize(width, height);
		return width;
	}

	float GetHeight()
	{
		float width, height;
		BuildMenuFilterButtonRoot.GetSize(width, height);
		return height;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == BuildMenuFilterButton && button == MouseState.LEFT) {
			m_Owner.OnFilterButtonPressed(this);
			return true;
		}

		return super.OnClick(w, x, y, button);
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (IsPartOfButton(w)) {
			m_IsHovered = true;
			UpdateVisualState();
		}

		return super.OnMouseEnter(w, x, y);
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (IsPartOfButton(w) && !IsPartOfButton(enterW)) {
			m_IsHovered = false;
			UpdateVisualState();
		}

		return super.OnMouseLeave(w, enterW, x, y);
	}

	override string GetLayoutFile()
	{
		return "DayZEditor/gui/layouts/buildmenu/EditorBuildMenuFilterButton.layout";
	}

	protected override bool UseUpdateLoop()
	{
		return false;
	}

	protected bool IsPartOfButton(Widget widget)
	{
		Widget current = widget;
		while (current) {
			if (current == GetLayoutRoot()) {
				return true;
			}

			current = current.GetParent();
		}

		return false;
	}
}

class EditorBuildMenuPreviewPool: Managed
{
	static const int MAX_ACTIVE_PREVIEWS = 64;
	protected ref array<ref EditorBuildMenuCard> m_ActiveCards = {};

	void Refresh(ScrollWidget scroll_widget, array<ref EditorBuildMenuCard> cards)
	{
		for (int i = m_ActiveCards.Count() - 1; i >= 0; i--) {
			EditorBuildMenuCard active_card = m_ActiveCards[i];
			if (!active_card || cards.Find(active_card) == -1 || !active_card.CanUsePreview(scroll_widget)) {
				if (active_card) {
					active_card.ReleasePreview();
				}

				m_ActiveCards.Remove(i);
			}
		}

		foreach (EditorBuildMenuCard card: cards) {
			if (!card) {
				continue;
			}

			if (m_ActiveCards.Find(card) != -1) {
				continue;
			}

			if (!card.CanUsePreview(scroll_widget)) {
				continue;
			}

			if (m_ActiveCards.Count() >= MAX_ACTIVE_PREVIEWS) {
				continue;
			}

			if (card.ActivatePreview()) {
				m_ActiveCards.Insert(card);
			}
		}

		foreach (EditorBuildMenuCard position_card: m_ActiveCards) {
			if (position_card) {
				position_card.UpdatePreviewPosition();
			}
		}
	}

	void Clear()
	{
		foreach (EditorBuildMenuCard active_card: m_ActiveCards) {
			if (active_card) {
				active_card.ReleasePreview();
			}
		}

		m_ActiveCards.Clear();
	}
}

class EditorBuildMenuWidgetFrame: Managed
{
	float X;
	float Y;
	float Width;
	float Height;

	void EditorBuildMenuWidgetFrame(float x = 0, float y = 0, float width = 0, float height = 0)
	{
		X = x;
		Y = y;
		Width = width;
		Height = height;
	}
}

class EditorBuildMenuCard: ScriptView
{
	static const string TITLE_ELLIPSIS = "...";

	protected EditorBuildMenuView m_Owner;
	protected EditorBuildMenuEntry m_Entry;
	protected EntityAI m_PreviewEntity;
	protected Widget m_PreviewOverlay;
	protected ItemPreviewWidget m_PreviewWidget;
	protected float m_BaseWidth;
	protected float m_BaseHeight;
	protected float m_Width;
	protected float m_Height;
	protected ref EditorBuildMenuWidgetFrame m_PreviewFrameLayout;
	protected ref EditorBuildMenuWidgetFrame m_FooterLayout;
	protected ref EditorBuildMenuWidgetFrame m_TitleLayout;
	protected ref EditorBuildMenuWidgetFrame m_SourceLayout;
	protected ref EditorBuildMenuWidgetFrame m_FavoriteLayout;
	protected ref EditorBuildMenuWidgetFrame m_FavoriteIconLayout;
	protected bool m_IsSelected;
	protected bool m_IsHovered;

	Widget BuildMenuCardRoot;
	ButtonWidget BuildMenuCardButton;
	Widget BuildMenuCardPreviewFrame;
	Widget BuildMenuCardFooter;
	ItemPreviewWidget BuildMenuCardPreview;
	TextWidget BuildMenuCardTitle;
	TextWidget BuildMenuCardSource;
	Widget BuildMenuCardFavorite;
	ButtonWidget BuildMenuCardFavoriteButton;
	ImageWidget BuildMenuCardFavoriteIcon;

	void EditorBuildMenuCard(EditorBuildMenuView owner, EditorBuildMenuEntry entry, bool selected)
	{
		m_Owner = owner;
		m_Entry = entry;
		m_IsSelected = selected;
		CaptureLayoutMetrics();
		BuildMenuCardFavoriteIcon.LoadImageFile(1, "set:dayz_editor_gui image:StarTicked");

		BuildMenuCardTitle.SetText(FitTitleToWidth(entry.DisplayName));
		BuildMenuCardSource.SetText(entry.SourceLabel);
		RefreshFavoriteState();
		if (BuildMenuCardPreview) {
			BuildMenuCardPreview.Show(false);
		}
		UpdateVisualState();
	}

	void ~EditorBuildMenuCard()
	{
		ReleasePreview();
	}

	EditorBuildMenuEntry GetEntry()
	{
		return m_Entry;
	}

	void SetPosition(float x, float y)
	{
		GetLayoutRoot().SetPos(x, y);
	}

	void SetCardWidth(float width)
	{
		if (m_BaseWidth <= 0 || m_BaseHeight <= 0) {
			CaptureLayoutMetrics();
		}

		if (width <= 0) {
			width = m_BaseWidth;
		}

		float scale = width / m_BaseWidth;
		m_Width = width;
		m_Height = m_BaseHeight * scale;

		GetLayoutRoot().SetSize(m_Width, m_Height);
		ApplyLayoutFrame(BuildMenuCardPreviewFrame, m_PreviewFrameLayout, scale);
		ApplyLayoutFrame(BuildMenuCardFooter, m_FooterLayout, scale);
		ApplyLayoutFrame(BuildMenuCardTitle, m_TitleLayout, scale);
		ApplyLayoutFrame(BuildMenuCardSource, m_SourceLayout, scale);
		ApplyLayoutFrame(BuildMenuCardFavorite, m_FavoriteLayout, scale);
		ApplyLayoutFrame(BuildMenuCardFavoriteIcon, m_FavoriteIconLayout, scale, false);
		BuildMenuCardTitle.SetText(FitTitleToWidth(m_Entry.DisplayName));
	}

	float GetBaseWidth()
	{
		return m_BaseWidth;
	}

	float GetBaseHeight()
	{
		return m_BaseHeight;
	}

	float GetWidth()
	{
		return m_Width;
	}

	float GetHeight()
	{
		return m_Height;
	}

	void SetSelected(bool selected)
	{
		m_IsSelected = selected;
		UpdateVisualState();
	}

	protected void UpdateVisualState()
	{
		if (BuildMenuCardRoot) {
			if (m_IsHovered) {
				BuildMenuCardRoot.SetColor(GetEditor().GetSettings().HighlightColor);
			} else {
				BuildMenuCardRoot.SetColor(ARGB(220, 28, 30, 34));
			}
		}

		if (BuildMenuCardFooter) {
			BuildMenuCardFooter.SetColor(GetFooterColor(m_IsSelected));
		}
	}

	bool CanUsePreview(ScrollWidget scroll_widget)
	{
		if (!m_Entry || !m_Entry.CanPreview || !GetLayoutRoot() || !BuildMenuCardPreviewFrame) {
			return false;
		}

		float scroll_x, scroll_y, scroll_w, scroll_h;
		scroll_widget.GetScreenPos(scroll_x, scroll_y);
		scroll_widget.GetScreenSize(scroll_w, scroll_h);

		float preview_x, preview_y, preview_w, preview_h;
		BuildMenuCardPreviewFrame.GetScreenPos(preview_x, preview_y);
		BuildMenuCardPreviewFrame.GetScreenSize(preview_w, preview_h);

		if (preview_w <= 0 || preview_h <= 0 || scroll_w <= 0 || scroll_h <= 0) {
			return true;
		}

		if (preview_y < scroll_y || preview_y + preview_h > scroll_y + scroll_h) {
			return false;
		}

		if (preview_x + preview_w < scroll_x || preview_x > scroll_x + scroll_w) {
			return false;
		}

		return true;
	}

	bool ActivatePreview()
	{
		if (m_PreviewEntity || !m_Entry || !m_Entry.CanPreview) {
			return m_PreviewEntity != null;
		}

		EditorPlaceableItem preview_placeable = m_Entry.PreviewPlaceable;
		if (!preview_placeable) {
			preview_placeable = m_Entry.Placeable;
		}

		Object preview_object = SpawnPreviewObject(preview_placeable.Type);
		if (!preview_object && preview_placeable.GetSpawnType() != preview_placeable.Type) {
			preview_object = SpawnPreviewObject(preview_placeable.GetSpawnType());
		}

		if (!preview_object) {
			string preview_type = GetEditor().GetObjectManager().ConvertP3dFileToPotentialObjectType(preview_placeable.GetModelName());
			if (preview_type == string.Empty && preview_placeable.Type.Contains(".p3d")) {
				preview_type = GetEditor().GetObjectManager().ConvertP3dFileToPotentialObjectType(preview_placeable.Type);
			}

			if (preview_type != string.Empty) {
				preview_object = SpawnPreviewObject(preview_type);
			}
		}

		// ItemPreviewWidget requires EntityAI, fall back to an EntityAI class when a raw preview object cannot be used directly.
		if (!Class.CastTo(m_PreviewEntity, preview_object)) {
			if (preview_object) {
				string fallback_type = GetEditor().GetObjectManager().ConvertP3dFileToPotentialObjectType(preview_object.GetShapeName());
				if (fallback_type != string.Empty) {
					Object fallback_object = SpawnPreviewObject(fallback_type);
					if (Class.CastTo(m_PreviewEntity, fallback_object)) {
						preview_object.Delete();
					} else if (fallback_object) {
						fallback_object.Delete();
					}
				}

				if (!m_PreviewEntity) {
					preview_object.Delete();
					return false;
				}
			} else {
				return false;
			}
		}

		// ItemPreviewWidget renders only from a root overlay.
		m_PreviewOverlay = GetGame().GetWorkspace().CreateWidgets("DayZEditor/gui/layouts/buildmenu/EditorBuildMenuPreviewOverlay.layout");
		if (m_PreviewOverlay) {
			m_PreviewWidget = ItemPreviewWidget.Cast(m_PreviewOverlay.FindAnyWidget("BuildMenuPreviewItem"));
			if (m_PreviewWidget) {
				PreparePreviewEntity(m_PreviewEntity);
				ConfigurePreviewWidget();
			}
			UpdatePreviewPosition();
		}

		return true;
	}

	protected Object SpawnPreviewObject(string preview_type)
	{
		if (preview_type == string.Empty) {
			return null;
		}

		return GetGame().CreateObjectEx(preview_type, Vector(0, -1000, 0), ECE_LOCAL | ECE_CREATEPHYSICS | ECE_TRACE);
	}

	protected void PreparePreviewEntity(EntityAI preview_entity)
	{
		if (!preview_entity) {
			return;
		}

		dBodyDestroy(preview_entity);
		preview_entity.DisableSimulation(true);
		preview_entity.SetAllowDamage(false);
	}

	protected void ConfigurePreviewWidget()
	{
		if (!m_PreviewWidget || !m_PreviewEntity) {
			return;
		}

		m_PreviewWidget.Show(true);
		m_PreviewWidget.SetItem(m_PreviewEntity);
		m_PreviewWidget.SetView(0);
		m_PreviewWidget.Update();
	}

	void UpdatePreviewPosition()
	{
		if (!m_PreviewOverlay || !BuildMenuCardPreviewFrame) {
			return;
		}

		float frame_x, frame_y, frame_w, frame_h;
		BuildMenuCardPreviewFrame.GetScreenPos(frame_x, frame_y);
		BuildMenuCardPreviewFrame.GetScreenSize(frame_w, frame_h);

		m_PreviewOverlay.SetPos(frame_x, frame_y);
		m_PreviewOverlay.SetSize(frame_w, frame_h);
	}

	void ReleasePreview()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdatePreviewPosition);

		if (m_PreviewWidget) {
			m_PreviewWidget.SetItem(null);
			m_PreviewWidget = null;
		}

		if (m_PreviewOverlay) {
			m_PreviewOverlay.Unlink();
			m_PreviewOverlay = null;
		}

		if (m_PreviewEntity) {
			m_PreviewEntity.Delete();
			m_PreviewEntity = null;
		}
	}

	protected int GetFooterColor(bool selected)
	{
		if (!m_Entry || !m_Entry.Placeable) {
			if (selected) {
				return ARGB(235, 46, 50, 58);
			}

			return ARGB(220, 36, 40, 46);
		}

		if (m_Entry.Placeable.Category == EditorPlaceableItemCategory.STATIC) {
			if (selected) {
				return ARGB(235, 58, 52, 44);
			}

			return ARGB(220, 46, 42, 36);
		}

		if (selected) {
			return ARGB(235, 46, 50, 58);
		}

		return ARGB(220, 36, 40, 46);
	}

	protected void CaptureLayoutMetrics()
	{
		if (!GetLayoutRoot()) {
			return;
		}

		GetLayoutRoot().GetSize(m_BaseWidth, m_BaseHeight);
		m_Width = m_BaseWidth;
		m_Height = m_BaseHeight;
		m_PreviewFrameLayout = ReadLayoutFrame(BuildMenuCardPreviewFrame);
		m_FooterLayout = ReadLayoutFrame(BuildMenuCardFooter);
		m_TitleLayout = ReadLayoutFrame(BuildMenuCardTitle);
		m_SourceLayout = ReadLayoutFrame(BuildMenuCardSource);
		m_FavoriteLayout = ReadLayoutFrame(BuildMenuCardFavorite);
		m_FavoriteIconLayout = ReadLayoutFrame(BuildMenuCardFavoriteIcon);
	}

	protected EditorBuildMenuWidgetFrame ReadLayoutFrame(Widget widget)
	{
		EditorBuildMenuWidgetFrame frame = new EditorBuildMenuWidgetFrame();
		if (!widget) {
			return frame;
		}

		widget.GetPos(frame.X, frame.Y);
		widget.GetSize(frame.Width, frame.Height);
		return frame;
	}

	protected void ApplyLayoutFrame(Widget widget, EditorBuildMenuWidgetFrame frame, float scale, bool apply_position = true)
	{
		if (!widget || !frame) {
			return;
		}

		if (apply_position) {
			widget.SetPos(frame.X * scale, frame.Y * scale);
		}

		widget.SetSize(frame.Width * scale, frame.Height * scale);
	}

	void RefreshFavoriteState()
	{
		if (!BuildMenuCardFavoriteIcon || !m_Entry) {
			return;
		}

		if (m_Entry.IsFavorite()) {
			BuildMenuCardFavoriteIcon.SetImage(1);
			BuildMenuCardFavoriteIcon.SetColor(LinearColor.GOLD);
		} else {
			BuildMenuCardFavoriteIcon.SetImage(0);
			BuildMenuCardFavoriteIcon.SetColor(LinearColor.WHITE);
		}
	}

	protected string FitTitleToWidth(string title)
	{
		if (!BuildMenuCardTitle || title == string.Empty) {
			return title;
		}

		title.TrimInPlace();
		if (title == string.Empty) {
			return title;
		}

		float width, height;
		BuildMenuCardTitle.GetSize(width, height);
		if (width <= 0) {
			width = m_TitleLayout.Width;
		}

		string candidate = title;
		if (DoesTextFit(BuildMenuCardTitle, candidate, width)) {
			return candidate;
		}

		string working_title = title;
		while (working_title.Length() > 1) {
			int trimmed_length = working_title.Length() - 1;
			if (trimmed_length <= 0) {
				break;
			}

			working_title = working_title.Substring(0, trimmed_length);
			working_title.TrimInPlace();
			if (working_title == string.Empty) {
				break;
			}

			candidate = working_title;
			candidate += TITLE_ELLIPSIS;
			if (DoesTextFit(BuildMenuCardTitle, candidate, width)) {
				return candidate;
			}
		}

		return TITLE_ELLIPSIS;
	}

	protected bool DoesTextFit(TextWidget widget, string text, float width)
	{
		if (!widget) {
			return false;
		}

		widget.SetText(text);
		widget.Update();

		int text_width, text_height;
		widget.GetTextSize(text_width, text_height);
		return text_width <= width;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == BuildMenuCardFavoriteButton && button == MouseState.LEFT) {
			bool favorite = !m_Entry.IsFavorite();
			m_Owner.OnCardFavoriteChanged(this, favorite);
			return true;
		}

		if (w == BuildMenuCardButton) {
			if (button == MouseState.LEFT) {
				m_Owner.SelectEntry(m_Entry);
				return true;
			}

			if (button == MouseState.RIGHT) {
				m_Owner.OpenEntryContextMenu(m_Entry, x, y);
				return true;
			}
		}

		return super.OnClick(w, x, y, button);
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (IsPartOfCard(w)) {
			m_IsHovered = true;
			UpdateVisualState();
		}

		return super.OnMouseEnter(w, x, y);
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (IsPartOfCard(w) && !IsPartOfCard(enterW)) {
			m_IsHovered = false;
			UpdateVisualState();
		}

		return super.OnMouseLeave(w, enterW, x, y);
	}

	override string GetLayoutFile()
	{
		return "DayZEditor/gui/layouts/buildmenu/EditorBuildMenuCard.layout";
	}

	protected override bool UseUpdateLoop()
	{
		return false;
	}

	protected bool IsPartOfCard(Widget widget)
	{
		Widget current = widget;
		while (current) {
			if (current == GetLayoutRoot()) {
				return true;
			}

			current = current.GetParent();
		}

		return false;
	}
}

class EditorBuildMenuSectionView: ScriptView
{
	static const float TITLE_HEIGHT = 22;
	static const float GRID_TOP = 30;
	static const float CARD_GAP = 12;
	static const int MAX_COLUMNS = 9;

	protected ref array<ref EditorBuildMenuCard> m_Cards = {};
	protected float m_Height;

	Widget BuildMenuSectionHeader;
	TextWidget BuildMenuSectionTitle;
	Widget BuildMenuSectionGrid;

	void EditorBuildMenuSectionView(EditorBuildMenuView owner, EditorBuildMenuSectionData section_data, EditorPlaceableItem selected_placeable, float available_width)
	{
		BuildMenuSectionTitle.SetText(section_data.Label);

		foreach (EditorBuildMenuEntry entry: section_data.Entries) {
			EditorBuildMenuCard card = new EditorBuildMenuCard(owner, entry, entry.Placeable == selected_placeable);
			m_Cards.Insert(card);
			BuildMenuSectionGrid.AddChild(card.GetLayoutRoot());
		}

		LayoutCards(available_width);
	}

	void ~EditorBuildMenuSectionView()
	{
		for (int i = m_Cards.Count() - 1; i >= 0; i--) {
			delete m_Cards[i];
		}
		m_Cards.Clear();
	}

	void AppendCards(out array<ref EditorBuildMenuCard> cards)
	{
		foreach (EditorBuildMenuCard card: m_Cards) {
			cards.Insert(card);
		}
	}

	void SetPosition(float x, float y)
	{
		GetLayoutRoot().SetPos(x, y);
	}

	float GetHeight()
	{
		return m_Height;
	}

	protected void LayoutCards(float available_width)
	{
		EditorBuildMenuCard sample_card = null;
		if (m_Cards.Count() > 0) {
			sample_card = m_Cards[0];
		}

		if (!sample_card) {
			m_Height = TITLE_HEIGHT;
			if (BuildMenuSectionHeader) {
				BuildMenuSectionHeader.SetSize(available_width, TITLE_HEIGHT);
			}
			BuildMenuSectionGrid.SetSize(available_width, 0);
			GetLayoutRoot().SetSize(available_width, m_Height);
			return;
		}

		float base_card_width = sample_card.GetBaseWidth();
		float base_card_height = sample_card.GetBaseHeight();
		if (base_card_width <= 0 || base_card_height <= 0) {
			return;
		}

		if (available_width < base_card_width) {
			available_width = base_card_width;
		}

		float card_width = base_card_width;
		float max_columns_width = MAX_COLUMNS * base_card_width + (MAX_COLUMNS - 1) * CARD_GAP;
		if (available_width < max_columns_width) {
			card_width = (available_width - (MAX_COLUMNS - 1) * CARD_GAP) / MAX_COLUMNS;
		}

		if (card_width <= 0) {
			card_width = base_card_width;
		}

		float card_height = base_card_height * (card_width / base_card_width);

		int columns = Math.Floor((available_width + CARD_GAP) / (card_width + CARD_GAP));
		if (columns < 1) {
			columns = 1;
		}
		if (columns > MAX_COLUMNS) {
			columns = MAX_COLUMNS;
		}

		for (int i = 0; i < m_Cards.Count(); i++) {
			EditorBuildMenuCard card = m_Cards[i];
			if (!card) {
				continue;
			}

			card.SetCardWidth(card_width);

			int column = i % columns;
			int row = i / columns;
			float x = column * (card_width + CARD_GAP);
			float y = row * (card_height + CARD_GAP);
			card.SetPosition(x, y);
		}

		int rows = 0;
		if (m_Cards.Count() > 0) {
			rows = (m_Cards.Count() + columns - 1) / columns;
		}

		float grid_height = 0;
		if (rows > 0) {
			grid_height = rows * card_height + (rows - 1) * CARD_GAP;
		}

		if (BuildMenuSectionHeader) {
			BuildMenuSectionHeader.SetSize(available_width, TITLE_HEIGHT);
		}

		BuildMenuSectionTitle.SetSize(available_width - 20, TITLE_HEIGHT);
		BuildMenuSectionGrid.SetSize(available_width, grid_height);
		m_Height = GRID_TOP + grid_height;
		if (grid_height <= 0) {
			m_Height = TITLE_HEIGHT;
		}

		GetLayoutRoot().SetSize(available_width, m_Height);
	}

	override string GetLayoutFile()
	{
		return "DayZEditor/gui/layouts/buildmenu/EditorBuildMenuSection.layout";
	}

	protected override bool UseUpdateLoop()
	{
		return false;
	}
}

class EditorBuildMenuView: ScriptView
{
	static const float FILTER_BUTTON_SPACING = 6;
	static const float FILTER_ROW_VERTICAL_PADDING = 4;
	static const float SECTION_SPACING = 20;
	static const int SEARCH_REFRESH_DELAY_MS = 200;
	static const int PERSISTENT_STATE_SAVE_DELAY_MS = 300;

	protected EditorHud m_EditorHud;
	protected Editor m_Editor;
	protected EditorBuildMenuCatalog m_Catalog;
	protected ref EditorBuildMenuFilterState m_FilterState = new EditorBuildMenuFilterState();
	protected ref EditorBuildMenuPreviewPool m_PreviewPool = new EditorBuildMenuPreviewPool();

	protected static const ref array<string> s_SourceFilterLabels = {
		"All",
		"Vanilla",
		"Mod"
	};
	protected ref array<ref EditorBuildMenuFilterButton> m_TabButtons = {};
	protected ref array<ref EditorBuildMenuFilterButton> m_SubcategoryButtons = {};
	protected ref array<ref EditorBuildMenuSectionView> m_SectionViews = {};

	protected bool m_IsOpen;
	protected bool m_RestoreCursorVisible;
	protected bool m_SuppressControlChanges;
	protected bool m_RenderQueued;
	protected bool m_ResetScrollOnNextRender = true;
	protected bool m_RebuildTabButtonsOnNextRender = true;
	protected bool m_RebuildSubcategoryButtonsOnNextRender = true;
	protected bool m_IsFavoritesOnlyHovered;
	protected bool m_IsConfigOnlyHovered;
	protected bool m_IsStaticOnlyHovered;
	protected float m_PreviewRefreshTimer;
	protected float m_LastScrollPosition = -1;
	protected string m_SearchText;
	protected string m_SavedTabId;
	protected string m_SavedSubcategoryId;
	protected int m_SavedSourceFilter;
	protected int m_SavedPlacementTypeFilter;
	protected bool m_SavedFavoritesOnly;

	Widget BuildMenuOverlay;
	EditBoxWidget BuildMenuSearch;
	ButtonWidget BuildMenuSearchClearButton;
	ButtonWidget BuildMenuCloseButton;
	ButtonWidget BuildMenuSourceCycleButton;
	TextWidget BuildMenuSourceCycleLabel;
	Widget BuildMenuTabRow;
	Widget BuildMenuSubcategoryRow;
	ScrollWidget BuildMenuScroll;
	Widget BuildMenuSectionList;
	Widget BuildMenuFavoritesOnlyRoot;
	ButtonWidget BuildMenuFavoritesOnlyButton;
	ImageWidget BuildMenuFavoritesOnlyIcon;
	Widget BuildMenuConfigOnlyRoot;
	ButtonWidget BuildMenuConfigOnlyButton;
	TextWidget BuildMenuConfigOnlyLabel;
	Widget BuildMenuStaticOnlyRoot;
	ButtonWidget BuildMenuStaticOnlyButton;
	TextWidget BuildMenuStaticOnlyLabel;
	TextWidget BuildMenuEmptyLabel;

	void EditorBuildMenuView(EditorHud editor_hud, EditorBuildMenuCatalog catalog)
	{
		m_EditorHud = editor_hud;
		m_Editor = GetEditor();
		m_Catalog = catalog;

		EditorSettings settings = m_Editor.GetSettings();
		m_FilterState.TabId = settings.BuildMenuLastTabId;
		m_FilterState.SubcategoryId = settings.BuildMenuLastSubtabId;
		m_FilterState.SourceFilter = Math.Clamp(settings.BuildMenuSourceFilter, 0, 2);
		m_FilterState.PlacementTypeFilter = Math.Clamp(settings.BuildMenuPlacementTypeFilter, 0, 1);
		m_FilterState.FavoritesOnly = settings.BuildMenuFavoritesOnly;
		m_FilterState.SetSearchText(m_SearchText);
		UpdateSavedStateSnapshot();

		UpdateFavoritesOnlyButton();
		UpdatePlacementTypeButtons();
		UpdateSearchClearButton();
		UpdateSourceCycleLabel();
		Show(false);
	}

	void ~EditorBuildMenuView()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(Render);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ApplySearchText);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(SavePersistentState);
		SavePersistentState();
		ClearTabButtons();
		ClearSubcategoryButtons();
		ClearSections();
		m_PreviewPool.Clear();
	}

	bool IsOpen()
	{
		return m_IsOpen;
	}

	void Open()
	{
		if (m_IsOpen) {
			return;
		}

		m_IsOpen = true;
		m_RestoreCursorVisible = GetGame().GetUIManager().IsCursorVisible();
		delete EditorHud.CurrentMenu;
		m_EditorHud.ClearCurrentTooltip();

		SyncToCurrentPlaceable();
		m_SuppressControlChanges = true;
		BuildMenuSearch.SetText(m_SearchText);
		m_SuppressControlChanges = false;
		UpdateFavoritesOnlyButton();
		UpdatePlacementTypeButtons();
		UpdateSearchClearButton();

		Show(true);
		GetGame().GetUIManager().ShowCursor(true);
		SetFocus(BuildMenuSearch);
		GetLayoutRoot().Update();
		Render();
	}

	void Close(bool restore_cursor = true)
	{
		if (!m_IsOpen) {
			return;
		}

		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ApplySearchText);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(SavePersistentState);
		m_IsOpen = false;
		m_SearchText = BuildMenuSearch.GetText();
		m_FilterState.SetSearchText(m_SearchText);
		SavePersistentState();
		m_PreviewPool.Clear();
		delete EditorHud.CurrentMenu;
		Show(false);
		SetFocus(null);

		if (restore_cursor) {
			m_EditorHud.ShowCursor(m_RestoreCursorVisible);
		}

		m_EditorHud.OnBuildMenuClosed();
	}

	override void Update(float dt)
	{
		super.Update(dt);

		if (!m_IsOpen) {
			return;
		}

		UAInput back_input = GetUApi().GetInputByName("UAUIBack");
		if (back_input && back_input.LocalPress()) {
			Close();
			return;
		}

		float scroll_position = BuildMenuScroll.GetVScrollPos();
		m_PreviewRefreshTimer -= dt;
		if (m_PreviewRefreshTimer <= 0 || scroll_position != m_LastScrollPosition) {
			RefreshPreviews();
		}
	}

	void OnFilterButtonPressed(EditorBuildMenuFilterButton button)
	{
		switch (button.GetButtonType()) {
			case EditorBuildMenuFilterButtonType.EDITOR_BUILD_MENU_FILTER_SOURCE: {
				SetSourceFilter(button.GetValue());
				break;
			}

			case EditorBuildMenuFilterButtonType.EDITOR_BUILD_MENU_FILTER_TAB: {
				SelectTab(button.GetId());
				break;
			}

			case EditorBuildMenuFilterButtonType.EDITOR_BUILD_MENU_FILTER_SUBCATEGORY: {
				SelectSubcategory(button.GetId());
				break;
			}
		}
	}

	void SelectEntry(EditorBuildMenuEntry entry)
	{
		if (!entry || !entry.Placeable) {
			return;
		}

		m_Editor.ClearHand();
		m_Editor.AddInHand(entry.Placeable);
		m_Editor.GetObjectManager().CurrentSelectedItem = entry.Placeable;
		Close();
	}

	void OpenEntryContextMenu(EditorBuildMenuEntry entry, int x, int y)
	{
		if (!entry || !entry.Placeable) {
			return;
		}

		m_Editor.GetObjectManager().CurrentSelectedItem = entry.Placeable;
		delete EditorHud.CurrentMenu;
		EditorHud.CurrentMenu = new EditorPlaceableContextMenu(x, y, entry.Placeable);
	}

	void OnCardFavoriteChanged(EditorBuildMenuCard card, bool favorite)
	{
		if (!card || !card.GetEntry() || !card.GetEntry().Placeable) {
			return;
		}

		m_EditorHud.SetFavoriteState(card.GetEntry().Placeable, favorite);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == BuildMenuCloseButton && button == MouseState.LEFT) {
			Close();
			return true;
		}

		if (w == BuildMenuSearchClearButton && button == MouseState.LEFT) {
			m_SuppressControlChanges = true;
			BuildMenuSearch.SetText(string.Empty);
			m_SuppressControlChanges = false;
			m_SearchText = string.Empty;
			UpdateSearchClearButton();
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ApplySearchText);
			ApplySearchText();
			SetFocus(BuildMenuSearch);
			return true;
		}

		if (w == BuildMenuFavoritesOnlyButton && button == MouseState.LEFT) {
			m_FilterState.FavoritesOnly = !m_FilterState.FavoritesOnly;
			UpdateFavoritesOnlyButton();
			QueuePersistentStateSave();
			QueueRender(false, false, false);
			return true;
		}

		if (w == BuildMenuConfigOnlyButton && button == MouseState.LEFT) {
			SetPlacementTypeFilter(EditorBuildMenuPlacementTypeFilter.EDITOR_BUILD_MENU_PLACEMENT_TYPE_CONFIG);
			return true;
		}

		if (w == BuildMenuStaticOnlyButton && button == MouseState.LEFT) {
			SetPlacementTypeFilter(EditorBuildMenuPlacementTypeFilter.EDITOR_BUILD_MENU_PLACEMENT_TYPE_STATIC);
			return true;
		}

		if (w == BuildMenuSourceCycleButton && button == MouseState.LEFT) {
			CycleSourceFilter();
			return true;
		}

		return super.OnClick(w, x, y, button);
	}

	override bool OnMouseWheel(Widget w, int x, int y, int wheel)
	{
		if (!m_IsOpen || !BuildMenuScroll || !IsDescendantOfBuildMenu(w)) {
			return super.OnMouseWheel(w, x, y, wheel);
		}

		BuildMenuScroll.VScrollStep(wheel * 14);
		RefreshPreviews();
		return true;
	}

	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (m_SuppressControlChanges) {
			return false;
		}

		if (w == BuildMenuSearch) {
			m_SearchText = BuildMenuSearch.GetText();
			UpdateSearchClearButton();
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ApplySearchText);
			if (finished) {
				ApplySearchText();
			} else {
				GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ApplySearchText, SEARCH_REFRESH_DELAY_MS, false);
			}
			return true;
		}

		return super.OnChange(w, x, y, finished);
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (IsPartOfFavoritesOnlyButton(w)) {
			m_IsFavoritesOnlyHovered = true;
			UpdateFavoritesOnlyButton();
		}

		if (IsPartOfConfigOnlyButton(w)) {
			m_IsConfigOnlyHovered = true;
			UpdatePlacementTypeButtons();
		}

		if (IsPartOfStaticOnlyButton(w)) {
			m_IsStaticOnlyHovered = true;
			UpdatePlacementTypeButtons();
		}

		return super.OnMouseEnter(w, x, y);
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (IsPartOfFavoritesOnlyButton(w) && !IsPartOfFavoritesOnlyButton(enterW)) {
			m_IsFavoritesOnlyHovered = false;
			UpdateFavoritesOnlyButton();
		}

		if (IsPartOfConfigOnlyButton(w) && !IsPartOfConfigOnlyButton(enterW)) {
			m_IsConfigOnlyHovered = false;
			UpdatePlacementTypeButtons();
		}

		if (IsPartOfStaticOnlyButton(w) && !IsPartOfStaticOnlyButton(enterW)) {
			m_IsStaticOnlyHovered = false;
			UpdatePlacementTypeButtons();
		}

		return super.OnMouseLeave(w, enterW, x, y);
	}

	protected void SelectTab(string tab_id)
	{
		if (m_FilterState.TabId == tab_id) {
			return;
		}

		m_FilterState.TabId = tab_id;
		m_FilterState.SubcategoryId = string.Empty;
		QueuePersistentStateSave();
		QueueRender(true, false, true);
	}

	protected void SelectSubcategory(string subcategory_id)
	{
		if (m_FilterState.SubcategoryId == subcategory_id) {
			return;
		}

		m_FilterState.SubcategoryId = subcategory_id;
		QueuePersistentStateSave();
		QueueRender(true, false, false);
	}

	protected void SetSourceFilter(int filter_value)
	{
		if (m_FilterState.SourceFilter == filter_value) {
			return;
		}

		m_FilterState.SourceFilter = filter_value;
		QueuePersistentStateSave();
		QueueRender(true, false, false);
	}

	protected void SetPlacementTypeFilter(int filter_value)
	{
		if (m_FilterState.PlacementTypeFilter == filter_value) {
			return;
		}

		m_FilterState.PlacementTypeFilter = filter_value;
		UpdatePlacementTypeButtons();
		QueuePersistentStateSave();
		QueueRender(true, false, false);
	}

	protected void Render()
	{
		m_RenderQueued = false;
		if (!m_IsOpen || !GetLayoutRoot()) {
			return;
		}

		string previous_tab_id = m_FilterState.TabId;
		bool rebuild_tab_buttons = m_RebuildTabButtonsOnNextRender || m_TabButtons.Count() == 0;
		bool rebuild_subcategory_buttons = m_RebuildSubcategoryButtonsOnNextRender || m_SubcategoryButtons.Count() == 0;
		m_RebuildTabButtonsOnNextRender = true;
		m_RebuildSubcategoryButtonsOnNextRender = true;

		m_PreviewPool.Clear();
		ClearSections();

		EnsureValidSelection();
		if (m_FilterState.TabId != previous_tab_id) {
			rebuild_subcategory_buttons = true;
		}

		if (rebuild_tab_buttons) {
			ClearTabButtons();
		}

		if (rebuild_subcategory_buttons) {
			ClearSubcategoryButtons();
		}

		QueuePersistentStateSave();
		GetLayoutRoot().Update();
		BuildMenuScroll.Update();
		UpdateSourceCycleLabel();
		UpdatePlacementTypeButtons();
		if (rebuild_tab_buttons) {
			BuildTabButtons();
		} else {
			UpdateTabButtonSelection();
		}
		if (rebuild_subcategory_buttons) {
			BuildSubcategoryButtons();
		} else {
			UpdateSubcategoryButtonSelection();
		}
		BuildSections();

		BuildMenuSectionList.Update();
		BuildMenuScroll.Update();
		if (m_ResetScrollOnNextRender) {
			BuildMenuScroll.VScrollToPos(0);
		}
		m_ResetScrollOnNextRender = true;
		RefreshPreviews();
	}

	protected void QueueRender(bool reset_scroll = true, bool rebuild_tab_buttons = true, bool rebuild_subcategory_buttons = true)
	{
		if (reset_scroll) {
			m_ResetScrollOnNextRender = true;
		} else if (!m_RenderQueued) {
			m_ResetScrollOnNextRender = false;
		}

		if (rebuild_tab_buttons) {
			m_RebuildTabButtonsOnNextRender = true;
		}

		if (rebuild_subcategory_buttons) {
			m_RebuildSubcategoryButtonsOnNextRender = true;
		}

		if (m_RenderQueued) {
			return;
		}

		m_RebuildTabButtonsOnNextRender = rebuild_tab_buttons;
		m_RebuildSubcategoryButtonsOnNextRender = rebuild_subcategory_buttons;
		m_RenderQueued = true;
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Render, 0, false);
	}

	protected void ApplySearchText()
	{
		m_FilterState.SetSearchText(m_SearchText);
		if (m_IsOpen) {
			QueueRender(false, false, false);
		}
	}

	protected void UpdateSourceCycleLabel()
	{
		int index = Math.Clamp(m_FilterState.SourceFilter, 0, s_SourceFilterLabels.Count() - 1);
		if (BuildMenuSourceCycleLabel) {
			BuildMenuSourceCycleLabel.SetText("Filter: " + s_SourceFilterLabels[index]);
		}
	}

	protected void CycleSourceFilter()
	{
		int next = (m_FilterState.SourceFilter + 1) % 3;
		SetSourceFilter(next);
		UpdateSourceCycleLabel();
	}

	protected void UpdateSearchClearButton()
	{
		if (BuildMenuSearchClearButton) {
			BuildMenuSearchClearButton.Show(m_SearchText != string.Empty);
		}
	}

	protected void UpdateFavoritesOnlyButton()
	{
		if (BuildMenuFavoritesOnlyRoot) {
			if (m_IsFavoritesOnlyHovered) {
				BuildMenuFavoritesOnlyRoot.SetColor(GetEditor().GetSettings().HighlightColor);
			} else {
				BuildMenuFavoritesOnlyRoot.SetColor(ARGB(255, 41, 43, 51));
			}
		}

		if (BuildMenuFavoritesOnlyIcon) {
			if (m_FilterState.FavoritesOnly) {
				BuildMenuFavoritesOnlyIcon.SetImage(3);
			} else {
				BuildMenuFavoritesOnlyIcon.SetImage(2);
			}

			if (m_FilterState.FavoritesOnly) {
				BuildMenuFavoritesOnlyIcon.SetColor(ARGB(255, 255, 255, 255));
			} else {
				BuildMenuFavoritesOnlyIcon.SetColor(ARGB(235, 201, 208, 216));
			}
		}
	}

	protected void UpdatePlacementTypeButtons()
	{
		UpdatePlacementTypeButton(BuildMenuConfigOnlyRoot, BuildMenuConfigOnlyLabel, m_IsConfigOnlyHovered, m_FilterState.PlacementTypeFilter == EditorBuildMenuPlacementTypeFilter.EDITOR_BUILD_MENU_PLACEMENT_TYPE_CONFIG);
		UpdatePlacementTypeButton(BuildMenuStaticOnlyRoot, BuildMenuStaticOnlyLabel, m_IsStaticOnlyHovered, m_FilterState.PlacementTypeFilter == EditorBuildMenuPlacementTypeFilter.EDITOR_BUILD_MENU_PLACEMENT_TYPE_STATIC);
	}

	protected void UpdatePlacementTypeButton(Widget root, TextWidget label, bool hovered, bool selected)
	{
		if (root) {
			if (selected) {
				root.SetColor(GetEditor().GetSettings().SelectionColor);
			} else if (hovered) {
				root.SetColor(GetEditor().GetSettings().HighlightColor);
			} else {
				root.SetColor(ARGB(255, 41, 43, 51));
			}
		}

		if (label) {
			if (selected || hovered) {
				label.SetColor(ARGB(255, 255, 255, 255));
			} else {
				label.SetColor(ARGB(235, 201, 208, 216));
			}
		}
	}

	protected void BuildTabButtons()
	{
		array<ref EditorBuildMenuTabDefinition> tabs = m_Catalog.GetTaxonomy().GetTabs();
		foreach (EditorBuildMenuTabDefinition tab: tabs) {
			CreateTabButton(tab.Id, tab.Label);
		}

		LayoutFilterButtons(m_TabButtons, BuildMenuTabRow);
	}

	protected void UpdateTabButtonSelection()
	{
		foreach (EditorBuildMenuFilterButton button: m_TabButtons) {
			if (button) {
				button.SetSelected(button.GetId() == m_FilterState.TabId);
			}
		}
	}

	protected void UpdateSubcategoryButtonSelection()
	{
		foreach (EditorBuildMenuFilterButton button: m_SubcategoryButtons) {
			if (button) {
				button.SetSelected(button.GetId() == m_FilterState.SubcategoryId);
			}
		}
	}

	protected void BuildSubcategoryButtons()
	{
		array<ref EditorBuildMenuSubcategoryDefinition> subcategories = m_Catalog.GetTaxonomy().GetSubcategories();
		foreach (EditorBuildMenuSubcategoryDefinition subcat: subcategories) {
			if (subcat.TabId == m_FilterState.TabId) {
				CreateSubcategoryButton(subcat.Id, subcat.Label);
			}
		}

		LayoutFilterButtons(m_SubcategoryButtons, BuildMenuSubcategoryRow);
	}

	protected void BuildSections()
	{
		EditorPlaceableItem selected_placeable = GetCurrentPlaceable();
		array<ref EditorBuildMenuSectionData> sections = m_Catalog.GetSections(m_FilterState);
		float content_width = GetSectionContentWidth();
		float current_y = 0;

		foreach (EditorBuildMenuSectionData section: sections) {
			EditorBuildMenuSectionView section_view = new EditorBuildMenuSectionView(this, section, selected_placeable, content_width);
			section_view.SetPosition(0, current_y);
			m_SectionViews.Insert(section_view);
			BuildMenuSectionList.AddChild(section_view.GetLayoutRoot());
			current_y += section_view.GetHeight() + SECTION_SPACING;
		}

		BuildMenuSectionList.SetSize(content_width, current_y);
		BuildMenuEmptyLabel.Show(sections.Count() == 0);
	}

	protected void EnsureValidSelection()
	{
		EditorBuildMenuFilterState tab_scope = CopyState();
		tab_scope.TabId = string.Empty;
		tab_scope.SubcategoryId = string.Empty;

		if (m_FilterState.TabId == string.Empty || m_Catalog.CountEntriesForTab(tab_scope, m_FilterState.TabId) == 0) {
			m_FilterState.TabId = GetFirstVisibleTab(tab_scope);
		}

		if (m_FilterState.TabId == string.Empty) {
			m_FilterState.SubcategoryId = string.Empty;
			return;
		}

		if (m_FilterState.SubcategoryId == string.Empty || m_Catalog.CountEntriesForSubcategory(tab_scope, m_FilterState.TabId, m_FilterState.SubcategoryId) == 0) {
			m_FilterState.SubcategoryId = GetFirstVisibleSubcategory(tab_scope, m_FilterState.TabId);
		}
	}

	protected string GetFirstVisibleTab(EditorBuildMenuFilterState scope_state)
	{
		array<ref EditorBuildMenuTabDefinition> tabs = m_Catalog.GetTaxonomy().GetTabs();
		foreach (EditorBuildMenuTabDefinition tab: tabs) {
			if (m_Catalog.CountEntriesForTab(scope_state, tab.Id) > 0) {
				return tab.Id;
			}
		}

		return string.Empty;
	}

	protected string GetFirstVisibleSubcategory(EditorBuildMenuFilterState scope_state, string tab_id)
	{
		array<ref EditorBuildMenuSubcategoryDefinition> subcategories = m_Catalog.GetTaxonomy().GetSubcategories();
		foreach (EditorBuildMenuSubcategoryDefinition subcat: subcategories) {
			if (subcat.TabId == tab_id && m_Catalog.CountEntriesForSubcategory(scope_state, tab_id, subcat.Id) > 0) {
				return subcat.Id;
			}
		}

		return string.Empty;
	}

	void RefreshAllCardFavorites()
	{
		foreach (EditorBuildMenuSectionView section_view: m_SectionViews) {
			ref array<ref EditorBuildMenuCard> section_cards = {};
			if (section_view) {
				section_view.AppendCards(section_cards);
				foreach (EditorBuildMenuCard card: section_cards) {
					if (card) {
						card.RefreshFavoriteState();
					}
				}
			}
		}
	}

	void OnFavoriteStateChanged()
	{
		if (m_IsOpen && m_FilterState.FavoritesOnly) {
			QueueRender(false, false, false);
		}
	}

	protected void RefreshPreviews()
	{
		ref array<ref EditorBuildMenuCard> cards = {};
		foreach (EditorBuildMenuSectionView section_view: m_SectionViews) {
			if (section_view) {
				section_view.AppendCards(cards);
			}
		}

		m_PreviewPool.Refresh(BuildMenuScroll, cards);
		m_PreviewRefreshTimer = 0.15;
		m_LastScrollPosition = BuildMenuScroll.GetVScrollPos();
	}

	protected void SyncToCurrentPlaceable()
	{
		EditorPlaceableItem current_placeable = GetCurrentPlaceable();
		if (!current_placeable) {
			return;
		}

		EditorBuildMenuEntry entry = m_Catalog.FindByPlaceable(current_placeable);
		if (!entry) {
			return;
		}

		m_FilterState.TabId = entry.TabId;
		m_FilterState.SubcategoryId = entry.SubcategoryId;
	}

	protected EditorPlaceableItem GetCurrentPlaceable()
	{
		if (m_Editor.IsPlacing()) {
			foreach (EditorWorldObject world_object: m_Editor.GetPlacingObjects()) {
				EditorHologram hologram;
				if (Class.CastTo(hologram, world_object)) {
					return hologram.GetPlaceableItem();
				}
			}
		}

		return m_Editor.GetObjectManager().CurrentSelectedItem;
	}

	protected EditorBuildMenuFilterState CopyState()
	{
		EditorBuildMenuFilterState state = new EditorBuildMenuFilterState();
		state.TabId = m_FilterState.TabId;
		state.SubcategoryId = m_FilterState.SubcategoryId;
		state.SourceFilter = m_FilterState.SourceFilter;
		state.PlacementTypeFilter = m_FilterState.PlacementTypeFilter;
		state.FavoritesOnly = m_FilterState.FavoritesOnly;
		state.SetSearchText(m_FilterState.SearchText);
		return state;
	}

	protected void SavePersistentState()
	{
		if (!IsPersistentStateChanged()) {
			return;
		}

		EditorSettings settings = m_Editor.GetSettings();
		settings.BuildMenuLastTabId = m_FilterState.TabId;
		settings.BuildMenuLastSubtabId = m_FilterState.SubcategoryId;
		settings.BuildMenuSourceFilter = m_FilterState.SourceFilter;
		settings.BuildMenuPlacementTypeFilter = m_FilterState.PlacementTypeFilter;
		settings.BuildMenuFavoritesOnly = m_FilterState.FavoritesOnly;
		settings.Save();
		UpdateSavedStateSnapshot();
	}

	protected void QueuePersistentStateSave()
	{
		if (!IsPersistentStateChanged()) {
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(SavePersistentState);
			return;
		}

		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(SavePersistentState);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(SavePersistentState, PERSISTENT_STATE_SAVE_DELAY_MS, false);
	}

	protected bool IsPersistentStateChanged()
	{
		if (m_FilterState.TabId != m_SavedTabId) {
			return true;
		}

		if (m_FilterState.SubcategoryId != m_SavedSubcategoryId) {
			return true;
		}

		if (m_FilterState.SourceFilter != m_SavedSourceFilter) {
			return true;
		}

		if (m_FilterState.PlacementTypeFilter != m_SavedPlacementTypeFilter) {
			return true;
		}

		if (m_FilterState.FavoritesOnly != m_SavedFavoritesOnly) {
			return true;
		}

		return false;
	}

	protected void UpdateSavedStateSnapshot()
	{
		m_SavedTabId = m_FilterState.TabId;
		m_SavedSubcategoryId = m_FilterState.SubcategoryId;
		m_SavedSourceFilter = m_FilterState.SourceFilter;
		m_SavedPlacementTypeFilter = m_FilterState.PlacementTypeFilter;
		m_SavedFavoritesOnly = m_FilterState.FavoritesOnly;
	}

	protected void LayoutFilterButtons(array<ref EditorBuildMenuFilterButton> buttons, Widget container)
	{
		if (!container) {
			return;
		}

		float x = 0;
		float row_height = 0;
		float y_offset = FILTER_ROW_VERTICAL_PADDING * 0.5;
		for (int i = 0; i < buttons.Count(); i++) {
			EditorBuildMenuFilterButton button = buttons[i];
			if (!button) {
				continue;
			}

			button.ResizeToContent();
			button.GetLayoutRoot().SetPos(x, y_offset);
			x += button.GetWidth() + FILTER_BUTTON_SPACING;

			float padded_height = button.GetHeight() + FILTER_ROW_VERTICAL_PADDING;
			if (padded_height > row_height) {
				row_height = padded_height;
			}
		}

		float width, height;
		container.GetSize(width, height);
		if (row_height <= 0) {
			row_height = height;
		}

		container.SetSize(width, row_height);
	}

	protected float GetSectionContentWidth()
	{
		float content_width = 0;
		float height = 0;

		if (BuildMenuTabRow) {
			BuildMenuTabRow.GetScreenSize(content_width, height);
			if (content_width <= 0) {
				BuildMenuTabRow.GetSize(content_width, height);
			}
		}

		if (content_width <= 0 && BuildMenuSubcategoryRow) {
			BuildMenuSubcategoryRow.GetScreenSize(content_width, height);
			if (content_width <= 0) {
				BuildMenuSubcategoryRow.GetSize(content_width, height);
			}
		}

		if (content_width > 0) {
			return content_width;
		}

		float scroll_width, scroll_height;
		BuildMenuScroll.GetScreenSize(scroll_width, scroll_height);
		if (scroll_width <= 0) {
			BuildMenuScroll.GetSize(scroll_width, scroll_height);
		}

		scroll_width -= 24;
		return scroll_width;
	}

	protected bool IsDescendantOfBuildMenu(Widget widget)
	{
		Widget current = widget;
		while (current) {
			if (current == BuildMenuOverlay) {
				return true;
			}

			current = current.GetParent();
		}

		return false;
	}

	protected bool IsPartOfFavoritesOnlyButton(Widget widget)
	{
		Widget current = widget;
		while (current) {
			if (current == BuildMenuFavoritesOnlyRoot) {
				return true;
			}

			current = current.GetParent();
		}

		return false;
	}

	protected bool IsPartOfConfigOnlyButton(Widget widget)
	{
		Widget current = widget;
		while (current) {
			if (current == BuildMenuConfigOnlyRoot) {
				return true;
			}

			current = current.GetParent();
		}

		return false;
	}

	protected bool IsPartOfStaticOnlyButton(Widget widget)
	{
		Widget current = widget;
		while (current) {
			if (current == BuildMenuStaticOnlyRoot) {
				return true;
			}

			current = current.GetParent();
		}

		return false;
	}

	protected void CreateTabButton(string id, string label)
	{
		EditorBuildMenuFilterButton button = new EditorBuildMenuFilterButton(this, label, EditorBuildMenuFilterButtonType.EDITOR_BUILD_MENU_FILTER_TAB, id);
		button.SetSelected(id == m_FilterState.TabId);
		m_TabButtons.Insert(button);
		BuildMenuTabRow.AddChild(button.GetLayoutRoot());
	}

	protected void CreateSubcategoryButton(string id, string label)
	{
		EditorBuildMenuFilterButton button = new EditorBuildMenuFilterButton(this, label, EditorBuildMenuFilterButtonType.EDITOR_BUILD_MENU_FILTER_SUBCATEGORY, id);
		button.SetSelected(id == m_FilterState.SubcategoryId);
		m_SubcategoryButtons.Insert(button);
		BuildMenuSubcategoryRow.AddChild(button.GetLayoutRoot());
	}



	protected void ClearTabButtons()
	{
		for (int i = m_TabButtons.Count() - 1; i >= 0; i--) {
			delete m_TabButtons[i];
		}

		m_TabButtons.Clear();
	}

	protected void ClearSubcategoryButtons()
	{
		for (int i = m_SubcategoryButtons.Count() - 1; i >= 0; i--) {
			delete m_SubcategoryButtons[i];
		}

		m_SubcategoryButtons.Clear();
	}

	protected void ClearSections()
	{
		for (int i = m_SectionViews.Count() - 1; i >= 0; i--) {
			delete m_SectionViews[i];
		}

		m_SectionViews.Clear();
	}

	override string GetLayoutFile()
	{
		return "DayZEditor/gui/layouts/buildmenu/EditorBuildMenu.layout";
	}
}
