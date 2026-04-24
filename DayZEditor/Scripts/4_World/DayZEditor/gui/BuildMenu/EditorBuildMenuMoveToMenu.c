class EditorBuildMenuCascadeMenuItem: EditorMenuItem
{
	protected ref EditorMenu m_ChildMenu;
	protected Widget Expand;
	protected ImageWidget Icon2;
	protected bool m_ExpandLeft;
	protected float m_LabelX;
	protected float m_LabelY;
	protected float m_ExpandX;
	protected float m_ExpandY;
	protected float m_ExpandWidth;

	void EditorBuildMenuCascadeMenuItem(notnull EditorMenu editor_menu, EditorCommand editor_command = null)
	{
		CacheLayoutMetrics();
		Shortcut.Show(false);
		if (Expand) {
			Expand.Show(true);
		}

		if (Icon2) {
			Icon2.LoadImageFile(0, "set:light image:arrow_right");
			Icon2.LoadImageFile(1, "set:light image:arrow_left");
			Icon2.SetImage(0);
		}
	}

	void ~EditorBuildMenuCascadeMenuItem()
	{
		delete m_ChildMenu;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (!IsInChildMenu(enterW)) {
			CloseChildMenu();
		}

		return super.OnMouseLeave(w, enterW, x, y);
	}

	override bool ClosesAfterInteraction()
	{
		return false;
	}

	void CloseChildMenu()
	{
		delete m_ChildMenu;
	}

	void SetExpandLeft(bool expand_left)
	{
		m_ExpandLeft = expand_left;
		float menu_width = GetParentMenuWidth();
		if (Expand) {
			if (m_ExpandLeft) {
				Expand.SetPos(menu_width - m_ExpandWidth - m_ExpandX, m_ExpandY);
			} else {
				Expand.SetPos(m_ExpandX, m_ExpandY);
			}
		}

		if (Icon2) {
			if (m_ExpandLeft) {
				Icon2.SetImage(1);
			} else {
				Icon2.SetImage(0);
			}
		}

		if (Label) {
			if (m_ExpandLeft) {
				Label.SetPos(m_ExpandX + m_ExpandWidth + 4, m_LabelY);
			} else {
				Label.SetPos(m_LabelX, m_LabelY);
			}
		}
	}

	protected void ShowChildMenu(EditorMenu child_menu)
	{
		delete m_ChildMenu;
		m_ChildMenu = child_menu;
		if (!m_ChildMenu) {
			return;
		}

		float x1, y1;
		float sx1, sy1;
		m_LayoutRoot.GetScreenPos(x1, y1);
		m_LayoutRoot.GetScreenSize(sx1, sy1);

		Widget child_root = m_ChildMenu.GetLayoutRoot();
		float child_width = GetMenuWidth(m_ChildMenu);
		float child_height = GetMenuHeight(m_ChildMenu);

		int screen_width, screen_height;
		GetScreenSize(screen_width, screen_height);

		float child_x;
		if (m_ExpandLeft) {
			child_x = x1 - child_width;
		} else {
			child_x = x1 + sx1;
		}

		float max_x = screen_width - child_width;
		if (max_x < 0) {
			max_x = 0;
		}

		float max_y = screen_height - child_height;
		if (max_y < 0) {
			max_y = 0;
		}

		child_root.SetPos(Math.Clamp(child_x, 0, max_x), Math.Clamp(y1, 0, max_y));
		child_root.Show(true);
	}

	protected bool IsInChildMenu(Widget widget)
	{
		if (!m_ChildMenu) {
			return false;
		}

		Widget child_root = m_ChildMenu.GetLayoutRoot();
		while (widget) {
			if (widget == child_root) {
				return true;
			}

			widget = widget.GetParent();
		}

		return false;
	}

	protected void CacheLayoutMetrics()
	{
		if (Label) {
			Label.GetPos(m_LabelX, m_LabelY);
		}

		if (Expand) {
			m_ExpandWidth = GetWidgetWidth(Expand);
			Expand.GetPos(m_ExpandX, m_ExpandY);
		}
	}

	protected float GetMenuWidth(EditorMenu menu)
	{
		if (!menu) {
			return GetParentMenuWidth();
		}

		float width, height;
		menu.GetLayoutRoot().GetScreenSize(width, height);
		if (width <= 0) {
			return GetParentMenuWidth();
		}

		return width;
	}

	protected float GetParentMenuWidth()
	{
		if (!m_ParentMenu) {
			return GetWidgetWidth(m_LayoutRoot);
		}

		float width = GetWidgetWidth(m_ParentMenu.GetLayoutRoot());
		if (width > 0) {
			return width;
		}

		return GetWidgetWidth(m_LayoutRoot);
	}

	protected float GetWidgetWidth(Widget widget)
	{
		if (!widget) {
			return 0;
		}

		float width, height;
		widget.GetScreenSize(width, height);
		if (width > 0) {
			return width;
		}

		widget.GetSize(width, height);
		return width;
	}

	protected float GetMenuHeight(EditorMenu menu)
	{
		if (!menu) {
			return 0;
		}

		float menu_height;
		ObservableCollection<ref EditorMenuItem> menu_items = menu.GetMenuItems();
		for (int i = 0; i < menu_items.Count(); i++) {
			EditorMenuItem menu_item = menu_items[i];
			if (!menu_item) {
				continue;
			}

			float item_width, item_height;
			menu_item.GetLayoutRoot().GetScreenSize(item_width, item_height);
			menu_height += item_height;
		}

		if (menu_height > 0) {
			return menu_height;
		}

		float width, height;
		menu.GetLayoutRoot().GetScreenSize(width, height);
		return height;
	}
}

class EditorBuildMenuMoveToMenuItem: EditorBuildMenuCascadeMenuItem
{
	protected EditorBuildMenuView m_BuildMenu;
	protected ref EditorBuildMenuEntry m_Entry;
	protected ref EditorBuildMenuSectionData m_Section;

	void EditorBuildMenuMoveToMenuItem(notnull EditorMenu editor_menu, EditorCommand editor_command = null, EditorBuildMenuView build_menu = null, EditorBuildMenuEntry entry = null, EditorBuildMenuSectionData section = null, float context_x = 0)
	{
		m_BuildMenu = build_menu;
		m_Entry = entry;
		m_Section = section;
		Label.SetText("Move To");
		SetExpandLeft(ShouldExpandLeft(context_x));
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_BuildMenu && m_BuildMenu.GetCatalog()) {
			ShowChildMenu(new EditorBuildMenuMoveTabsMenu(m_BuildMenu, m_Entry, m_Section, m_ExpandLeft));
		}

		return super.OnMouseEnter(w, x, y);
	}

	protected bool ShouldExpandLeft(float context_x)
	{
		int screen_width, screen_height;
		GetScreenSize(screen_width, screen_height);

		float menu_width = GetParentMenuWidth();
		float root_x = context_x;
		if (root_x > screen_width - menu_width) {
			root_x -= menu_width;
		}

		float max_root_x = screen_width - menu_width;
		if (max_root_x < 0) {
			max_root_x = 0;
		}

		root_x = Math.Clamp(root_x, 0, max_root_x);
		float chain_width = menu_width * 2;
		float available_right = screen_width - (root_x + menu_width);
		float available_left = root_x;

		if (chain_width <= available_right) {
			return false;
		}

		if (chain_width <= available_left) {
			return true;
		}

		return available_left > available_right;
	}
}

class EditorBuildMenuMoveTabsMenu: EditorMenu
{
	protected EditorBuildMenuView m_BuildMenu;
	protected ref EditorBuildMenuEntry m_Entry;
	protected ref EditorBuildMenuSectionData m_Section;
	protected EditorBuildMenuMoveTabMenuItem m_ActiveItem;
	protected bool m_ExpandLeft;

	void EditorBuildMenuMoveTabsMenu(EditorBuildMenuView build_menu, EditorBuildMenuEntry entry = null, EditorBuildMenuSectionData section = null, bool expand_left = false)
	{
		m_BuildMenu = build_menu;
		m_Entry = entry;
		m_Section = section;
		m_ExpandLeft = expand_left;

		CreateTabItems();
	}

	void SetActiveItem(EditorBuildMenuMoveTabMenuItem active_item)
	{
		if (m_ActiveItem && m_ActiveItem != active_item) {
			m_ActiveItem.CloseChildMenu();
		}

		m_ActiveItem = active_item;
	}

	protected void CreateTabItems()
	{
		if (!m_BuildMenu || !m_BuildMenu.GetCatalog()) {
			return;
		}

		array<ref EditorBuildMenuTabDefinition> tabs = m_BuildMenu.GetCatalog().GetTaxonomy().GetTabs();
		foreach (EditorBuildMenuTabDefinition tab_definition: tabs) {
			if (!tab_definition) {
				continue;
			}

			AddMenuItem(new EditorBuildMenuMoveTabMenuItem(this, null, m_BuildMenu, m_Entry, m_Section, tab_definition.Id, tab_definition.Label, m_ExpandLeft));
		}
	}
}

class EditorBuildMenuMoveTabMenuItem: EditorBuildMenuCascadeMenuItem
{
	protected EditorBuildMenuView m_BuildMenu;
	protected ref EditorBuildMenuEntry m_Entry;
	protected ref EditorBuildMenuSectionData m_Section;
	protected string m_TabId;

	void EditorBuildMenuMoveTabMenuItem(notnull EditorMenu editor_menu, EditorCommand editor_command = null, EditorBuildMenuView build_menu = null, EditorBuildMenuEntry entry = null, EditorBuildMenuSectionData section = null, string tab_id = string.Empty, string label = string.Empty, bool expand_left = false)
	{
		m_BuildMenu = build_menu;
		m_Entry = entry;
		m_Section = section;
		m_TabId = tab_id;
		Label.SetText(label);
		SetExpandLeft(expand_left);
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		EditorBuildMenuMoveTabsMenu parent_menu = EditorBuildMenuMoveTabsMenu.Cast(m_ParentMenu);
		if (parent_menu) {
			parent_menu.SetActiveItem(this);
		}

		if (m_BuildMenu && m_BuildMenu.GetCatalog() && m_TabId != string.Empty) {
			ShowChildMenu(new EditorBuildMenuMoveSubcategoriesMenu(m_BuildMenu, m_Entry, m_Section, m_TabId));
		}

		return super.OnMouseEnter(w, x, y);
	}
}

class EditorBuildMenuMoveSubcategoriesMenu: EditorMenu
{
	protected EditorBuildMenuView m_BuildMenu;
	protected ref EditorBuildMenuEntry m_Entry;
	protected ref EditorBuildMenuSectionData m_Section;
	protected string m_TabId;

	void EditorBuildMenuMoveSubcategoriesMenu(EditorBuildMenuView build_menu, EditorBuildMenuEntry entry = null, EditorBuildMenuSectionData section = null, string tab_id = string.Empty)
	{
		m_BuildMenu = build_menu;
		m_Entry = entry;
		m_Section = section;
		m_TabId = tab_id;

		CreateSubcategoryItems();
	}

	protected void CreateSubcategoryItems()
	{
		if (!m_BuildMenu || !m_BuildMenu.GetCatalog() || m_TabId == string.Empty) {
			return;
		}

		array<ref EditorBuildMenuSubcategoryDefinition> subcategories = m_BuildMenu.GetCatalog().GetTaxonomy().GetSubcategories();
		foreach (EditorBuildMenuSubcategoryDefinition subcategory_definition: subcategories) {
			if (!subcategory_definition || subcategory_definition.TabId != m_TabId) {
				continue;
			}

			AddMenuItem(new EditorBuildMenuMoveSubcategoryMenuItem(this, GetEditor().CommandManager[EditorBuildMenuMoveToCommand], m_BuildMenu, m_Entry, m_Section, m_TabId, subcategory_definition.Id, subcategory_definition.Label));
		}
	}
}

class EditorBuildMenuMoveSubcategoryMenuItem: EditorMenuItem
{
	protected EditorBuildMenuView m_BuildMenu;
	protected ref EditorBuildMenuEntry m_Entry;
	protected ref EditorBuildMenuSectionData m_Section;
	protected string m_TabId;
	protected string m_SubcategoryId;

	void EditorBuildMenuMoveSubcategoryMenuItem(notnull EditorMenu editor_menu, EditorCommand editor_command = null, EditorBuildMenuView build_menu = null, EditorBuildMenuEntry entry = null, EditorBuildMenuSectionData section = null, string tab_id = string.Empty, string subcategory_id = string.Empty, string label = string.Empty)
	{
		m_BuildMenu = build_menu;
		m_Entry = entry;
		m_Section = section;
		m_TabId = tab_id;
		m_SubcategoryId = subcategory_id;
		Label.SetText(label);
	}

	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (m_Command && button == MouseState.LEFT) {
			m_Command.SetData(new Param5<EditorBuildMenuView, EditorBuildMenuEntry, EditorBuildMenuSectionData, string, string>(m_BuildMenu, m_Entry, m_Section, m_TabId, m_SubcategoryId));
		}

		return super.OnMouseButtonDown(w, x, y, button);
	}
}
