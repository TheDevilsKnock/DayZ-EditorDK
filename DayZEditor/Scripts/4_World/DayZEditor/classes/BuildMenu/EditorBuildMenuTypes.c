enum EditorBuildMenuPlatformFilter
{
	EDITOR_BUILD_MENU_PLATFORM_FILTER_PC = 0,
	EDITOR_BUILD_MENU_PLATFORM_FILTER_CONSOLE = 1
}

enum EditorBuildMenuPlacementTypeFilter
{
	EDITOR_BUILD_MENU_PLACEMENT_TYPE_CONFIG = 0,
	EDITOR_BUILD_MENU_PLACEMENT_TYPE_STATIC = 1
}

class DayZEditorBuildMenuTabRegistration: Managed
{
	string Id;
	string Label;
	string Icon;
	int Order = -1;

	void DayZEditorBuildMenuTabRegistration(string id, string label, string icon = string.Empty, int order = -1)
	{
		Id = id;
		Label = label;
		Icon = icon;
		Order = order;
	}
}

class DayZEditorBuildMenuSubcategoryRegistration: Managed
{
	string TabId;
	string Id;
	string Label;
	string Icon;
	int Order = -1;

	void DayZEditorBuildMenuSubcategoryRegistration(string tab_id, string id, string label, string icon = string.Empty, int order = -1)
	{
		TabId = tab_id;
		Id = id;
		Label = label;
		Icon = icon;
		Order = order;
	}
}

class DayZEditorBuildMenuSectionRegistration: Managed
{
	string TabId;
	string SubcategoryId;
	string Id;
	string Label;
	int Order = -1;

	void DayZEditorBuildMenuSectionRegistration(string tab_id, string subcategory_id, string id, string label, int order = -1)
	{
		TabId = tab_id;
		SubcategoryId = subcategory_id;
		Id = id;
		Label = label;
		Order = order;
	}
}

class DayZEditorBuildMenuEntryRegistration: Managed
{
	int MatchType;
	string MatchValue;
	string TabId;
	string SubcategoryId;
	string SectionId;
	int SourceKind;
	string SourceLabel;
	int Priority = 100;

	void DayZEditorBuildMenuEntryRegistration(int match_type, string match_value, string tab_id, string subcategory_id, string section_id = string.Empty, int source_kind = DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED, string source_label = string.Empty, int priority = 100)
	{
		MatchType = match_type;
		MatchValue = match_value;
		TabId = tab_id;
		SubcategoryId = subcategory_id;
		SectionId = section_id;
		SourceKind = source_kind;
		SourceLabel = source_label;
		Priority = priority;
	}
}

class EditorBuildMenuTabDefinition: Managed
{
	string Id;
	string Label;
	string Icon;
	int Order;

	void EditorBuildMenuTabDefinition(string id, string label, int order, string icon = string.Empty)
	{
		Id = id;
		Label = label;
		Order = order;
		Icon = icon;
	}
}

class EditorBuildMenuSubcategoryDefinition: Managed
{
	string TabId;
	string Id;
	string Label;
	string Icon;
	int Order;

	void EditorBuildMenuSubcategoryDefinition(string tab_id, string id, string label, int order, string icon = string.Empty)
	{
		TabId = tab_id;
		Id = id;
		Label = label;
		Order = order;
		Icon = icon;
	}
}

class EditorBuildMenuSectionDefinition: Managed
{
	string TabId;
	string SubcategoryId;
	string Id;
	string Label;
	int Order;

	void EditorBuildMenuSectionDefinition(string tab_id, string subcategory_id, string id, string label, int order)
	{
		TabId = tab_id;
		SubcategoryId = subcategory_id;
		Id = id;
		Label = label;
		Order = order;
	}
}

class EditorBuildMenuMatchResult: Managed
{
	string TabId;
	string SubcategoryId;
	string SectionId;
	string SourceId;
	string SourceLabel;
	bool Resolved;
}

class EditorBuildMenuSourceData: Managed
{
	string Id;
	string Label;

	void EditorBuildMenuSourceData(string id, string label)
	{
		Id = id;
		Label = label;
	}
}

class EditorBuildMenuFilterState: Managed
{
	string TabId;
	string SubcategoryId;
	string SearchText;
	int PlatformFilter = EditorBuildMenuPlatformFilter.EDITOR_BUILD_MENU_PLATFORM_FILTER_PC;
	int PlacementTypeFilter = EditorBuildMenuPlacementTypeFilter.EDITOR_BUILD_MENU_PLACEMENT_TYPE_CONFIG;
	bool FavoritesOnly;
	ref array<string> SelectedSourceIds = {};

	void SetSearchText(string search_text)
	{
		SearchText = search_text;
		SearchText.ToLower();
	}
}

class EditorBuildMenuEntry: Managed
{
	EditorPlaceableItem Placeable;
	EditorPlaceableItem PreviewPlaceable;
	string StableId;
	string DisplayName;
	string SearchText;
	string TabId;
	string SubcategoryId;
	string SectionId;
	string SectionLabel;
	string SourceId;
	string SourceLabel;
	string SortKey;
	bool CanPreview;

	void EditorBuildMenuEntry(EditorPlaceableItem placeable, EditorBuildMenuMatchResult match_result, string display_name, EditorBuildMenuTaxonomy taxonomy)
	{
		Placeable = placeable;
		PreviewPlaceable = placeable;
		StableId = EditorBuildMenuCatalog.GetStableId(placeable);
		DisplayName = display_name;
		TabId = match_result.TabId;
		SubcategoryId = match_result.SubcategoryId;
		SectionId = match_result.SectionId;
		SourceId = match_result.SourceId;
		SourceLabel = match_result.SourceLabel;
		CanPreview = EditorBuildMenuInference.CanPreview(placeable, match_result);

		if (taxonomy) {
			SectionLabel = taxonomy.GetSectionLabel(TabId, SubcategoryId, SectionId);
		} else {
			SectionLabel = EditorBuildMenuInference.FormatLabel(match_result.SectionId);
		}

		SortKey = DisplayName;
		SortKey.ToLower();

		SearchText = string.Format("%1 %2 %3 %4 %5 %6 %7 %8", DisplayName, placeable.Type, placeable.Name, placeable.Path, TabId, SubcategoryId, SectionLabel, SourceLabel);
		SearchText.ToLower();
	}

	bool IsFavorite()
	{
		return Placeable && Placeable.IsFavorite();
	}

	void SetPreviewPlaceable(EditorPlaceableItem placeable)
	{
		if (placeable) {
			PreviewPlaceable = placeable;
		}
	}

	bool Matches(EditorBuildMenuFilterState state, string tab_override = string.Empty, string subcategory_override = string.Empty)
	{
		if (tab_override != string.Empty && TabId != tab_override) {
			return false;
		}

		if (subcategory_override != string.Empty && SubcategoryId != subcategory_override) {
			return false;
		}

		if (state) {
			if (state.TabId != string.Empty && TabId != state.TabId) {
				return false;
			}

			if (state.SubcategoryId != string.Empty && SubcategoryId != state.SubcategoryId) {
				return false;
			}

			if (state.PlatformFilter == EditorBuildMenuPlatformFilter.EDITOR_BUILD_MENU_PLATFORM_FILTER_CONSOLE) {
				if (!Placeable || !Placeable.ConsoleFriendly) {
					return false;
				}
			}

			if (state.SelectedSourceIds && state.SelectedSourceIds.Count() > 0 && state.SelectedSourceIds.Find(SourceId) == -1) {
				return false;
			}

			if (Placeable && Placeable.Category != EditorPlaceableItemCategory.SCRIPTED) {
				if (state.PlacementTypeFilter == EditorBuildMenuPlacementTypeFilter.EDITOR_BUILD_MENU_PLACEMENT_TYPE_CONFIG) {
					if (Placeable.Category != EditorPlaceableItemCategory.CONFIG) {
						return false;
					}
				} else {
					if (Placeable.Category != EditorPlaceableItemCategory.STATIC) {
						return false;
					}
				}
			}

			if (state.FavoritesOnly && !IsFavorite()) {
				return false;
			}

			if (state.SearchText != string.Empty && !SearchText.Contains(state.SearchText)) {
				return false;
			}
		}

		return true;
	}
}

class EditorBuildMenuSectionData: Managed
{
	string Id;
	string Label;
	int Order = -1;
	ref array<ref EditorBuildMenuEntry> Entries = {};

	void EditorBuildMenuSectionData(string id, string label, int order = -1)
	{
		Id = id;
		Label = label;
		Order = order;
	}
}
