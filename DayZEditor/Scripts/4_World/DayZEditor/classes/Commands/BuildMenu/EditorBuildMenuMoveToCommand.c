[RegisterEditorCommand(EditorBuildMenuMoveToCommand)]
class EditorBuildMenuMoveToCommand: EditorCommand
{
	protected override bool Execute(Class sender, CommandArgs args)
	{
		super.Execute(sender, args);
		if (!m_Editor) {
			return false;
		}

		Param5<EditorBuildMenuView, EditorBuildMenuEntry, EditorBuildMenuSectionData, string, string> data = Param5<EditorBuildMenuView, EditorBuildMenuEntry, EditorBuildMenuSectionData, string, string>.Cast(GetData());
		if (!data || !data.param1 || data.param4 == string.Empty || data.param5 == string.Empty) {
			return false;
		}

		if (data.param3) {
			return MoveSection(data.param1, data.param3, data.param4, data.param5);
		}

		if (data.param2) {
			return MoveEntry(data.param1, data.param2, data.param4, data.param5);
		}

		return false;
	}

	protected bool MoveEntry(EditorBuildMenuView build_menu, EditorBuildMenuEntry entry, string tab_id, string subcategory_id)
	{
		if (!build_menu || !entry) {
			return false;
		}

		// TODO: Persist asset reclassification through the source-owned BuildMenu XML pack.
		EditorLog.Info("TODO EditorBuildMenuMoveToCommand::MoveEntry %1 -> %2/%3", entry.StableId, tab_id, subcategory_id);
		m_Editor.GetEditorHud().CreateNotification("BuildMenu asset reclassification is not implemented yet.", 3.0, LinearColor.YELLOW);
		return true;
	}

	protected bool MoveSection(EditorBuildMenuView build_menu, EditorBuildMenuSectionData section, string tab_id, string subcategory_id)
	{
		if (!build_menu || !section) {
			return false;
		}

		// TODO: Persist section reclassification through the source-owned BuildMenu XML pack.
		EditorLog.Info("TODO EditorBuildMenuMoveToCommand::MoveSection %1 -> %2/%3", section.Id, tab_id, subcategory_id);
		m_Editor.GetEditorHud().CreateNotification("BuildMenu section reclassification is not implemented yet.", 3.0, LinearColor.YELLOW);
		return true;
	}

	override string GetName()
	{
		return "Move To";
	}
}
