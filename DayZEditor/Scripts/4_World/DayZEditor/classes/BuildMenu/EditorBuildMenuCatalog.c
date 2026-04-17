class EditorBuildMenuCatalog: Managed
{
	protected ref EditorBuildMenuTaxonomy m_Taxonomy;
	protected ref array<ref EditorBuildMenuEntry> m_Entries = {};
	protected ref map<string, ref EditorBuildMenuEntry> m_EntriesByStableId = new map<string, ref EditorBuildMenuEntry>();
	protected ref map<string, ref array<ref EditorBuildMenuEntry>> m_EntriesByTab = new map<string, ref array<ref EditorBuildMenuEntry>>();
	protected ref map<string, ref array<ref EditorBuildMenuEntry>> m_EntriesBySubcategory = new map<string, ref array<ref EditorBuildMenuEntry>>();

	static string GetStableId(EditorPlaceableItem placeable)
	{
		return string.Format("%1|%2|%3", placeable.Category, placeable.Type, placeable.Path);
	}

	void Build(array<ref EditorPlaceableItem> placeable_items)
	{
		m_Taxonomy = EditorBuildMenuInference.CreateTaxonomy();
		m_Entries.Clear();
		m_EntriesByStableId.Clear();
		m_EntriesByTab.Clear();
		m_EntriesBySubcategory.Clear();

		ref map<string, int> entry_keys = new map<string, int>();

		foreach (EditorPlaceableItem placeable_item: placeable_items) {
			if (!placeable_item) {
				continue;
			}

			string display_name = EditorBuildMenuInference.GetDisplayName(placeable_item);
			EditorBuildMenuMatchResult match_result = EditorBuildMenuInference.Classify(placeable_item);
			if (!match_result || !match_result.Resolved) {
				continue;
			}

			EditorBuildMenuEntry entry = new EditorBuildMenuEntry(placeable_item, match_result, display_name, m_Taxonomy);
			string entry_key = GetDedupeKey(entry);
			if (entry_key != string.Empty && entry_keys.Contains(entry_key)) {
				int existing_index = entry_keys[entry_key];
				EditorBuildMenuEntry existing_entry = m_Entries[existing_index];
				m_EntriesByStableId[entry.StableId] = existing_entry;
				continue;
			}

			m_Entries.Insert(entry);
			m_EntriesByStableId[entry.StableId] = entry;
			entry_keys[entry_key] = m_Entries.Count() - 1;
		}

		IndexEntries();
	}

	EditorBuildMenuTaxonomy GetTaxonomy()
	{
		if (!m_Taxonomy) {
			m_Taxonomy = EditorBuildMenuInference.CreateTaxonomy();
		}

		return m_Taxonomy;
	}

	EditorBuildMenuEntry FindByPlaceable(EditorPlaceableItem placeable)
	{
		if (!placeable) {
			return null;
		}

		return m_EntriesByStableId[GetStableId(placeable)];
	}

	array<ref EditorBuildMenuSectionData> GetSections(EditorBuildMenuFilterState state)
	{
		ref array<ref EditorBuildMenuSectionData> sections = {};
		ref map<string, ref EditorBuildMenuSectionData> sections_by_id = new map<string, ref EditorBuildMenuSectionData>();
		array<ref EditorBuildMenuEntry> candidates = GetCandidateEntries(state);
		if (!candidates) {
			return sections;
		}

		foreach (EditorBuildMenuEntry entry: candidates) {
			if (!entry || !entry.Matches(state)) {
				continue;
			}

			EditorBuildMenuSectionData section = sections_by_id[entry.SectionId];
			if (!section) {
				section = new EditorBuildMenuSectionData(entry.SectionId, m_Taxonomy.GetSectionLabel(entry.TabId, entry.SubcategoryId, entry.SectionId), m_Taxonomy.GetSectionOrder(entry.TabId, entry.SubcategoryId, entry.SectionId));
				InsertSectionSorted(sections, section);
				sections_by_id[entry.SectionId] = section;
			}

			InsertEntrySorted(section.Entries, entry);
		}

		return sections;
	}

	int CountEntriesForTab(EditorBuildMenuFilterState state, string tab_id)
	{
		array<ref EditorBuildMenuEntry> candidates = m_EntriesByTab[tab_id];
		if (!candidates) {
			return 0;
		}

		int count = 0;
		foreach (EditorBuildMenuEntry entry: candidates) {
			if (entry && entry.Matches(state, tab_id)) {
				count++;
			}
		}

		return count;
	}

	int CountEntriesForSubcategory(EditorBuildMenuFilterState state, string tab_id, string subcategory_id)
	{
		array<ref EditorBuildMenuEntry> candidates = m_EntriesBySubcategory[GetSubcategoryKey(tab_id, subcategory_id)];
		if (!candidates) {
			return 0;
		}

		int count = 0;
		foreach (EditorBuildMenuEntry entry: candidates) {
			if (entry && entry.Matches(state, tab_id, subcategory_id)) {
				count++;
			}
		}

		return count;
	}

	bool HasMatches(EditorBuildMenuFilterState state)
	{
		array<ref EditorBuildMenuEntry> candidates = GetCandidateEntries(state);
		if (!candidates) {
			return false;
		}

		foreach (EditorBuildMenuEntry entry: candidates) {
			if (entry && entry.Matches(state)) {
				return true;
			}
		}

		return false;
	}

	protected void IndexEntries()
	{
		foreach (EditorBuildMenuEntry entry: m_Entries) {
			if (!entry) {
				continue;
			}

			EnsureBucket(m_EntriesByTab, entry.TabId).Insert(entry);
			EnsureBucket(m_EntriesBySubcategory, GetSubcategoryKey(entry.TabId, entry.SubcategoryId)).Insert(entry);
		}
	}

	protected array<ref EditorBuildMenuEntry> EnsureBucket(map<string, ref array<ref EditorBuildMenuEntry>> buckets, string key)
	{
		if (!buckets[key]) {
			buckets[key] = {};
		}

		return buckets[key];
	}

	protected array<ref EditorBuildMenuEntry> GetCandidateEntries(EditorBuildMenuFilterState state)
	{
		if (state) {
			if (state.TabId != string.Empty && state.SubcategoryId != string.Empty) {
				return m_EntriesBySubcategory[GetSubcategoryKey(state.TabId, state.SubcategoryId)];
			}

			if (state.TabId != string.Empty) {
				return m_EntriesByTab[state.TabId];
			}
		}

		return m_Entries;
	}

	protected static void InsertEntrySorted(array<ref EditorBuildMenuEntry> entries, EditorBuildMenuEntry entry)
	{
		int insert_index = entries.Count();
		for (int i = 0; i < entries.Count(); i++) {
			if (entry.SortKey < entries[i].SortKey) {
				insert_index = i;
				break;
			}
		}

		entries.InsertAt(entry, insert_index);
	}

	protected static void InsertSectionSorted(array<ref EditorBuildMenuSectionData> sections, EditorBuildMenuSectionData section)
	{
		int insert_index = sections.Count();
		for (int i = 0; i < sections.Count(); i++) {
			if (SortSectionBefore(section, sections[i])) {
				insert_index = i;
				break;
			}
		}

		sections.InsertAt(section, insert_index);
	}

	protected static bool SortSectionBefore(EditorBuildMenuSectionData lhs, EditorBuildMenuSectionData rhs)
	{
		if ((lhs.Order >= 0) != (rhs.Order >= 0)) {
			return lhs.Order >= 0;
		}

		if (lhs.Order != rhs.Order && lhs.Order >= 0 && rhs.Order >= 0) {
			return lhs.Order < rhs.Order;
		}

		bool lhs_fallback = IsFallbackSection(lhs.Id);
		bool rhs_fallback = IsFallbackSection(rhs.Id);
		if (lhs_fallback != rhs_fallback) {
			return !lhs_fallback;
		}

		string lhs_label = lhs.Label;
		string rhs_label = rhs.Label;
		lhs_label.ToLower();
		rhs_label.ToLower();
		return lhs_label < rhs_label;
	}

	protected static bool IsFallbackSection(string section_id)
	{
		return section_id == "unknown" || section_id == "misc";
	}

	protected static string GetDedupeKey(EditorBuildMenuEntry entry)
	{
		if (!entry || !entry.Placeable) {
			return string.Empty;
		}

		switch (entry.Placeable.Category) {
			case EditorPlaceableItemCategory.CONFIG: {
				string type_lower = entry.Placeable.Type;
				type_lower.ToLower();
				return "config|" + type_lower;
			}

			case EditorPlaceableItemCategory.STATIC: {
				string normalized_model = EditorBuildMenuInference.NormalizeDedupePath(entry.Placeable.GetModelName());
				if (normalized_model == string.Empty) {
					normalized_model = EditorBuildMenuInference.NormalizeDedupePath(entry.Placeable.Path);
				}

				if (normalized_model == string.Empty) {
					return string.Empty;
				}

				return "static|" + normalized_model;
			}

			case EditorPlaceableItemCategory.SCRIPTED: {
				string type_lower_scripted = entry.Placeable.Type;
				type_lower_scripted.ToLower();
				return "scripted|" + type_lower_scripted;
			}
		}

		return string.Empty;
	}

	protected string GetSubcategoryKey(string tab_id, string subcategory_id)
	{
		return string.Format("%1|%2", tab_id, subcategory_id);
	}
}
