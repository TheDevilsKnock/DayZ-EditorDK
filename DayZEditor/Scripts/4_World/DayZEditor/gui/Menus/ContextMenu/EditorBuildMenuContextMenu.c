class EditorBuildMenuContextMenu: EditorContextMenu
{
	void EditorBuildMenuContextMenu(float x, float y, EditorBuildMenuView build_menu, EditorBuildMenuEntry entry = null, EditorBuildMenuSectionData section = null)
	{
		AddMenuItem(new EditorBuildMenuMoveToMenuItem(this, null, build_menu, entry, section, x));

		if (entry && entry.Placeable) {
			AddMenuDivider();
			AddMenuButton(m_Editor.CommandManager[EditorLootEditorCommand]);

			m_Editor.CommandManager[EditorCopyPlaceableToClipboard].SetData(new Param1<EditorPlaceableItem>(entry.Placeable));
			AddMenuButton(m_Editor.CommandManager[EditorCopyPlaceableToClipboard]);
		}
	}
}
