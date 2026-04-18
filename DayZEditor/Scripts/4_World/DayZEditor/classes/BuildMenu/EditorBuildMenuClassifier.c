class EditorBuildMenuPathRule: Managed
{
	string Path;
	string TabId;
	string SubcategoryId;
	string SectionId;
	int SourceKind = DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED;
	string SourceId;
	string SourceLabel;
	int Order = -1;

	void EditorBuildMenuPathRule(string path, string tab_id, string subcategory_id, string section_id = string.Empty, int source_kind = DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED, string source_id = string.Empty, string source_label = string.Empty, int order = -1)
	{
		Path = path;
		TabId = tab_id;
		SubcategoryId = subcategory_id;
		SectionId = section_id;
		SourceKind = source_kind;
		SourceId = source_id;
		SourceLabel = source_label;
		Order = order;
	}
}

class XMLEditorBuildMenuClassification: XMLCallback
{
	protected ref array<ref DayZEditorBuildMenuSectionRegistration> m_Sections;
	protected ref array<ref EditorBuildMenuPathRule> m_Paths;
	protected bool m_Success;
	protected bool m_HasSchemaError;
	protected bool m_HasVanillaPaths;

	void XMLEditorBuildMenuClassification(array<ref DayZEditorBuildMenuSectionRegistration> sections, array<ref EditorBuildMenuPathRule> paths)
	{
		m_Sections = sections;
		m_Paths = paths;
	}

	override void OnSuccess(XMLDocument document)
	{
		m_Success = false;
		m_HasSchemaError = false;
		m_HasVanillaPaths = false;

		XMLTag root = document.Get(1);
		if (!root) {
			EditorLog.Error("XMLEditorBuildMenuClassification::OnSuccess root tag missing");
			return;
		}

		XMLElement content = root.GetContent();
		for (int i = 0; i < content.Count(); i++) {
			XMLTag child_tag = content.Get(i);
			if (!child_tag) {
				continue;
			}

			switch (child_tag.GetName()) {
				case "VanillaPaths":
					m_HasVanillaPaths = true;
					ParseScopedPaths(child_tag.GetContent(), DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_VANILLA, "vanilla", "Vanilla");
					break;

				case "Mods":
					ParseMods(child_tag.GetContent());
					break;

				case "Sections":
				case "Paths":
					MarkDeprecatedFlatSchema(child_tag.GetName());
					break;

				default:
					EditorLog.Warning("XMLEditorBuildMenuClassification::OnSuccess ignoring unsupported root tag %1", child_tag.GetName());
					break;
			}
		}

		if (!m_HasVanillaPaths) {
			EditorLog.Error("XMLEditorBuildMenuClassification::OnSuccess missing required VanillaPaths block");
			return;
		}

		if (m_HasSchemaError) {
			return;
		}

		m_Success = true;
	}

	override void OnFailure(XMLDocument document)
	{
		EditorLog.Error("XMLEditorBuildMenuClassification::OnFailure");
	}

	bool IsSuccessful()
	{
		return m_Success;
	}

	protected void ParseMods(XMLElement content)
	{
		for (int i = 0; i < content.Count(); i++) {
			XMLTag mod_tag = content.Get(i);
			if (!mod_tag || mod_tag.GetName() != "Mod") {
				continue;
			}

			string mod_id = EditorBuildMenuInference.NormalizeId(GetStringAttribute(mod_tag, "id"));
			if (mod_id == string.Empty) {
				EditorLog.Error("XMLEditorBuildMenuClassification::ParseMods missing required mod id");
				m_HasSchemaError = true;
				continue;
			}

			string mod_label = GetStringAttribute(mod_tag, "label");
			if (mod_label == string.Empty && mod_id != string.Empty) {
				mod_label = EditorBuildMenuInference.FormatLabel(mod_id);
			}

			if (mod_label == string.Empty) {
				mod_label = "Modded";
			}

			ParseScopedPaths(mod_tag.GetContent(), DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED, mod_id, mod_label);
		}
	}

	protected void ParseScopedPaths(XMLElement content, int source_kind, string source_id, string source_label)
	{
		for (int i = 0; i < content.Count(); i++) {
			XMLTag tab_tag = content.Get(i);
			if (!tab_tag) {
				continue;
			}

			if (tab_tag.GetName() != "Tab") {
				MarkDeprecatedFlatSchema(tab_tag.GetName());
				continue;
			}

			string tab_id = EditorBuildMenuInference.NormalizeId(GetStringAttribute(tab_tag, "id"));
			if (tab_id == string.Empty) {
				EditorLog.Error("XMLEditorBuildMenuClassification::ParseScopedPaths missing tab id");
				continue;
			}

			if (!EditorBuildMenuTaxonomy.IsBuiltinTabId(tab_id)) {
				EditorLog.Error("XMLEditorBuildMenuClassification::ParseScopedPaths invalid tab %1", tab_id);
				m_HasSchemaError = true;
				continue;
			}

			ParseTabPaths(tab_tag.GetContent(), tab_id, source_kind, source_id, source_label);
		}
	}

	protected void ParseTabPaths(XMLElement content, string tab_id, int source_kind, string source_id, string source_label)
	{
		for (int i = 0; i < content.Count(); i++) {
			XMLTag subcategory_tag = content.Get(i);
			if (!subcategory_tag) {
				continue;
			}

			if (subcategory_tag.GetName() != "Subcategory") {
				EditorLog.Error("XMLEditorBuildMenuClassification::ParseTabPaths invalid child %1 under tab %2; v3 requires Subcategory nodes", subcategory_tag.GetName(), tab_id);
				m_HasSchemaError = true;
				continue;
			}

			string subcategory_id = EditorBuildMenuInference.NormalizeId(GetStringAttribute(subcategory_tag, "id"));
			if (subcategory_id == string.Empty) {
				EditorLog.Error("XMLEditorBuildMenuClassification::ParseTabPaths missing subcategory id under tab %1", tab_id);
				continue;
			}

			if (!EditorBuildMenuTaxonomy.IsBuiltinSubcategoryId(tab_id, subcategory_id)) {
				EditorLog.Error("XMLEditorBuildMenuClassification::ParseTabPaths invalid target %1/%2", tab_id, subcategory_id);
				m_HasSchemaError = true;
				continue;
			}

			ParseSubcategoryPaths(subcategory_tag.GetContent(), tab_id, subcategory_id, source_kind, source_id, source_label);
		}
	}

	protected void ParseSubcategoryPaths(XMLElement content, string tab_id, string subcategory_id, int source_kind, string source_id, string source_label)
	{
		for (int i = 0; i < content.Count(); i++) {
			XMLTag child_tag = content.Get(i);
			if (!child_tag) {
				continue;
			}

			switch (child_tag.GetName()) {
				case "Sections":
					ParseLocalSections(child_tag.GetContent(), tab_id, subcategory_id);
					break;

				case "Path":
					ParseLocalPath(child_tag, tab_id, subcategory_id, source_kind, source_id, source_label);
					break;

				default:
					EditorLog.Error("XMLEditorBuildMenuClassification::ParseSubcategoryPaths invalid child %1 under %2/%3", child_tag.GetName(), tab_id, subcategory_id);
					m_HasSchemaError = true;
					break;
			}
		}
	}

	protected void ParseLocalSections(XMLElement content, string tab_id, string subcategory_id)
	{
		for (int i = 0; i < content.Count(); i++) {
			XMLTag section_tag = content.Get(i);
			if (!section_tag) {
				continue;
			}

			if (section_tag.GetName() != "Section") {
				EditorLog.Error("XMLEditorBuildMenuClassification::ParseLocalSections invalid child %1 under %2/%3", section_tag.GetName(), tab_id, subcategory_id);
				m_HasSchemaError = true;
				continue;
			}

			string section_id = EditorBuildMenuInference.NormalizeId(GetStringAttribute(section_tag, "id"));
			string section_label = GetStringAttribute(section_tag, "label");
			int section_order = GetIntAttribute(section_tag, "order", -1);
			if (section_id == string.Empty) {
				EditorLog.Error("XMLEditorBuildMenuClassification::ParseLocalSections missing section id for %1/%2", tab_id, subcategory_id);
				continue;
			}

			m_Sections.Insert(new DayZEditorBuildMenuSectionRegistration(tab_id, subcategory_id, section_id, section_label, section_order));
		}
	}

	protected void ParseLocalPath(XMLTag path_tag, string tab_id, string subcategory_id, int source_kind, string source_id, string source_label)
	{
		string folder_path = EditorBuildMenuInference.NormalizeTextPath(GetStringAttribute(path_tag, "folder"));
		string section_id = EditorBuildMenuInference.NormalizeId(GetStringAttribute(path_tag, "section"));
		int order = GetIntAttribute(path_tag, "order", -1);

		if (folder_path == string.Empty) {
			EditorLog.Error("XMLEditorBuildMenuClassification::ParseLocalPath missing folder for %1/%2", tab_id, subcategory_id);
			return;
		}

		if (GetStringAttribute(path_tag, "tab") != string.Empty || GetStringAttribute(path_tag, "subcategory") != string.Empty || GetStringAttribute(path_tag, "source") != string.Empty) {
			EditorLog.Error("XMLEditorBuildMenuClassification::ParseLocalPath deprecated flat attributes found on %1; v3 uses inherited Tab/Subcategory context", folder_path);
			m_HasSchemaError = true;
			return;
		}

		m_Paths.Insert(new EditorBuildMenuPathRule(folder_path, tab_id, subcategory_id, section_id, source_kind, source_id, source_label, order));
	}

	protected void MarkDeprecatedFlatSchema(string tag_name)
	{
		EditorLog.Error("XMLEditorBuildMenuClassification::MarkDeprecatedFlatSchema deprecated v2 tag %1 found; v3 requires Tab/Subcategory/Path nesting", tag_name);
		m_HasSchemaError = true;
	}

	protected string GetStringAttribute(XMLTag tag, string name, string default_value = string.Empty)
	{
		XMLAttribute attribute = tag.GetAttribute(name);
		if (!attribute) {
			return default_value;
		}

		return attribute.ValueAsString();
	}

	protected int GetIntAttribute(XMLTag tag, string name, int default_value = -1)
	{
		XMLAttribute attribute = tag.GetAttribute(name);
		if (!attribute) {
			return default_value;
		}

		return attribute.ValueAsInt();
	}
}

class EditorBuildMenuInference
{
	protected static const string CLASSIFICATION_XML_FILE = "DayZEditor/Scripts/Data/Defaults/BuildMenuClassification.xml";
	protected static const ref array<string> NOISE_SEGMENTS = {
		"assets",
		"data",
		"proxy",
		"proxies",
		"animations",
		"animconfig"
	};

	protected static ref array<ref DayZEditorBuildMenuSectionRegistration> s_SectionDefinitions;
	protected static ref array<ref DayZEditorBuildMenuSectionRegistration> s_RegisteredSections;
	protected static ref array<ref DayZEditorBuildMenuEntryRegistration> s_RegisteredEntries;
	protected static ref array<ref EditorBuildMenuPathRule> s_PathRules;
	protected static bool s_DefinitionsLoaded;

	static EditorBuildMenuTaxonomy CreateTaxonomy()
	{
		EnsureDefinitionsLoaded();

		EditorBuildMenuTaxonomy taxonomy = new EditorBuildMenuTaxonomy();
		foreach (DayZEditorBuildMenuSectionRegistration section_definition: s_SectionDefinitions) {
			taxonomy.AddSection(section_definition.TabId, section_definition.SubcategoryId, section_definition.Id, section_definition.Label, section_definition.Order);
		}

		foreach (DayZEditorBuildMenuSectionRegistration section_registration: s_RegisteredSections) {
			taxonomy.AddSection(section_registration.TabId, section_registration.SubcategoryId, section_registration.Id, section_registration.Label, section_registration.Order);
		}

		taxonomy.FinalizeDefinitions();
		return taxonomy;
	}

	static string GetDisplayName(EditorPlaceableItem placeable)
	{
		if (!placeable) {
			return string.Empty;
		}

		if (placeable.Category != EditorPlaceableItemCategory.STATIC) {
			return placeable.Type;
		}

		if (placeable.Name != string.Empty) {
			return placeable.Name;
		}

		string model_name = placeable.GetModelName();
		if (model_name == string.Empty) {
			model_name = placeable.Path;
		}

		model_name.Replace("\\", "/");
		model_name.Replace(SystemPath.SEPERATOR_ALT, "/");
		model_name.Replace(SystemPath.SEPERATOR, "/");

		int last_separator = model_name.LastIndexOf("/");
		if (last_separator != -1 && last_separator + 1 < model_name.Length()) {
			return model_name.Substring(last_separator + 1, model_name.Length() - last_separator - 1);
		}

		if (model_name != string.Empty) {
			return model_name;
		}

		return placeable.Type;
	}

	static bool CanPreview(EditorPlaceableItem placeable, EditorBuildMenuMatchResult match_result)
	{
		if (!placeable) {
			return false;
		}

		if (match_result) {
			if (match_result.TabId == "effects") {
				return false;
			}

			if (match_result.SubcategoryId == "particles" || match_result.SubcategoryId == "scripted") {
				return false;
			}
		}

		if (GetGame().IsKindOf(placeable.Type, "Man")) {
			return false;
		}

		if (GetGame().IsKindOf(placeable.Type, "DZ_LightAI")) {
			return false;
		}

		if (GetGame().IsKindOf(placeable.Type, "PointLightBase")) {
			return false;
		}

		if (GetGame().IsKindOf(placeable.Type, "ScriptedLightBase")) {
			return false;
		}

		return true;
	}

	static EditorBuildMenuMatchResult Classify(EditorPlaceableItem placeable)
	{
		EnsureDefinitionsLoaded();

		string normalized_path = NormalizePreviewPath(placeable);
		EditorBuildMenuMatchResult registered_result = MatchRegisteredEntry(placeable, normalized_path);
		if (registered_result) {
			return registered_result;
		}

		EditorBuildMenuMatchResult scripted_result = MatchScripted(placeable);
		if (scripted_result) {
			return scripted_result;
		}

		EditorBuildMenuPathRule matched_rule = FindPathRule(normalized_path);
		if (matched_rule) {
			return CreateResult(matched_rule.TabId, matched_rule.SubcategoryId, GetSectionId(normalized_path, matched_rule), GetSourceKind(placeable, normalized_path, matched_rule.SourceId, matched_rule.SourceKind, matched_rule.SourceLabel), GetSourceId(placeable, normalized_path, matched_rule.SourceId, matched_rule.SourceKind, matched_rule.SourceLabel), GetSourceLabel(placeable, normalized_path, matched_rule.SourceId, matched_rule.SourceKind, matched_rule.SourceLabel));
		}

		EditorBuildMenuMatchResult actor_result = MatchActorOrEffect(placeable, normalized_path);
		if (actor_result) {
			return actor_result;
		}

		string fallback_section = GetFallbackSection(normalized_path);
		if (fallback_section == string.Empty) {
			fallback_section = "unknown";
		}

		string type_lower = placeable.Type;
		type_lower.ToLower();
		if (type_lower.IndexOf("land_") == 0 || type_lower.IndexOf("staticobj_") == 0 || type_lower.IndexOf("bldr_") == 0) {
			return CreateResult("structures", "misc", fallback_section, GetSourceKind(placeable, normalized_path), GetSourceId(placeable, normalized_path), GetSourceLabel(placeable, normalized_path));
		}

		return CreateResult("misc", "unknown", fallback_section, GetSourceKind(placeable, normalized_path), GetSourceId(placeable, normalized_path), GetSourceLabel(placeable, normalized_path));
	}

	static string NormalizePreviewPath(EditorPlaceableItem placeable)
	{
		if (!placeable) {
			return string.Empty;
		}

		string path = placeable.GetModelName();
		if (path == string.Empty) {
			path = placeable.Path;
		}

		return NormalizeTextPath(path);
	}

	static string NormalizeDedupePath(string path)
	{
		if (path == string.Empty) {
			return string.Empty;
		}

		path.Replace("\\", "/");
		path.Replace(SystemPath.SEPERATOR_ALT, "/");
		path.Replace(SystemPath.SEPERATOR, "/");
		path.TrimInPlace();
		path.ToLower();

		while (path.Contains("//")) {
			path.Replace("//", "/");
		}

		while (path.Length() > 0 && path.Substring(0, 1) == "/") {
			path = path.Substring(1, path.Length() - 1);
		}

		return path;
	}

	static string NormalizeTextPath(string path)
	{
		path = NormalizeDedupePath(path);
		if (path == string.Empty) {
			return string.Empty;
		}

		path.Replace("structures_bliss/", "structures/");
		path.Replace("structures_sakhal/", "structures/");
		path.Replace("plants_bliss/", "plants/");
		path.Replace("plants_sakhal/", "plants/");
		path.Replace("rocks_bliss/", "rocks/");
		path.Replace("rocks_sakhal/", "rocks/");
		path.Replace("water_bliss/", "water/");
		path.Replace("water_sakhal/", "water/");
		return path;
	}

	static string NormalizeId(string value)
	{
		if (value == string.Empty) {
			return string.Empty;
		}

		value.Replace("-", "_");
		value.Replace(" ", "_");
		value.ToLower();
		return value;
	}

	static string FormatLabel(string value)
	{
		if (value == string.Empty) {
			return "Unknown";
		}

		string formatted = StripExtension(value);
		formatted.Replace("_", " ");
		formatted.Replace("-", " ");
		formatted.Replace("/", " ");
		formatted.TrimInPlace();

		array<string> words = {};
		formatted.Split(" ", words);

		string result = string.Empty;
		foreach (string word: words) {
			word.TrimInPlace();
			if (word == string.Empty) {
				continue;
			}

			if (word.Length() > 1) {
				string first_char = word.Substring(0, 1);
				first_char.ToUpper();
				word = first_char + word.Substring(1, word.Length() - 1);
			} else {
				word.ToUpper();
			}

			if (result != string.Empty) {
				result += " ";
			}

			result += word;
		}

		if (result == string.Empty) {
			return "Unknown";
		}

		return result;
	}

	protected static void EnsureDefinitionsLoaded()
	{
		if (s_DefinitionsLoaded) {
			return;
		}

		s_DefinitionsLoaded = true;
		s_SectionDefinitions = {};
		s_RegisteredSections = GetRegisteredSections();
		s_RegisteredEntries = GetRegisteredEntries();
		s_PathRules = {};

		string file_name = SystemPath.Format(CLASSIFICATION_XML_FILE);
		if (!File.Exists(file_name)) {
			EditorLog.Error("EditorBuildMenuInference::EnsureDefinitionsLoaded file missing %1", file_name);
			return;
		}

		XMLEditorBuildMenuClassification xml_definition = new XMLEditorBuildMenuClassification(s_SectionDefinitions, s_PathRules);
		GetXMLApi().Read(file_name, xml_definition);
		if (!xml_definition.IsSuccessful()) {
			EditorLog.Error("EditorBuildMenuInference::EnsureDefinitionsLoaded failed to parse %1", file_name);
			return;
		}

		SortPathRules();
	}

	protected static void SortPathRules()
	{
		ref array<ref EditorBuildMenuPathRule> sorted = {};
		foreach (EditorBuildMenuPathRule path_rule: s_PathRules) {
			int insert_index = sorted.Count();
			for (int i = 0; i < sorted.Count(); i++) {
				if (SortPathRuleBefore(path_rule, sorted[i])) {
					insert_index = i;
					break;
				}
			}

			sorted.InsertAt(path_rule, insert_index);
		}

		s_PathRules = sorted;
	}

	protected static array<ref DayZEditorBuildMenuSectionRegistration> GetRegisteredSections()
	{
		ref array<ref DayZEditorBuildMenuSectionRegistration> registrations = {};
		if (!RegisterDayZEditorBuildMenuSection.Instances) {
			return registrations;
		}

		foreach (Param5<string, string, string, string, int> registration_data: RegisterDayZEditorBuildMenuSection.Instances) {
			if (!registration_data) {
				continue;
			}

			registrations.Insert(new DayZEditorBuildMenuSectionRegistration(registration_data.param1, registration_data.param2, registration_data.param3, registration_data.param4, registration_data.param5));
		}

		return registrations;
	}

	protected static array<ref DayZEditorBuildMenuEntryRegistration> GetRegisteredEntries()
	{
		ref array<ref DayZEditorBuildMenuEntryRegistration> registrations = {};
		if (!RegisterDayZEditorBuildMenuEntry.Instances) {
			return registrations;
		}

		foreach (Param8<int, string, string, string, string, int, string, int> registration_data: RegisterDayZEditorBuildMenuEntry.Instances) {
			if (!registration_data) {
				continue;
			}

			registrations.Insert(new DayZEditorBuildMenuEntryRegistration(registration_data.param1, registration_data.param2, registration_data.param3, registration_data.param4, registration_data.param5, registration_data.param6, registration_data.param7, registration_data.param8));
		}

		return registrations;
	}

	protected static bool SortPathRuleBefore(EditorBuildMenuPathRule lhs, EditorBuildMenuPathRule rhs)
	{
		if (lhs.Path.Length() != rhs.Path.Length()) {
			return lhs.Path.Length() > rhs.Path.Length();
		}

		if ((lhs.Order >= 0) != (rhs.Order >= 0)) {
			return lhs.Order >= 0;
		}

		if (lhs.Order != rhs.Order && lhs.Order >= 0 && rhs.Order >= 0) {
			return lhs.Order < rhs.Order;
		}

		return lhs.Path < rhs.Path;
	}

	protected static EditorBuildMenuPathRule FindPathRule(string normalized_path)
	{
		if (normalized_path == string.Empty) {
			return null;
		}

		// The longest prefix wins so child folders can override parent folders.
		foreach (EditorBuildMenuPathRule path_rule: s_PathRules) {
			if (MatchesPathPrefix(normalized_path, path_rule.Path)) {
				return path_rule;
			}
		}

		return null;
	}

	protected static bool MatchesPathPrefix(string normalized_path, string prefix)
	{
		if (prefix == string.Empty || normalized_path == string.Empty) {
			return false;
		}

		if (normalized_path == prefix) {
			return true;
		}

		if (normalized_path.IndexOf(prefix) != 0) {
			return false;
		}

		if (normalized_path.Length() <= prefix.Length()) {
			return false;
		}

		return normalized_path.Substring(prefix.Length(), 1) == "/";
	}

	protected static string GetSectionId(string normalized_path, EditorBuildMenuPathRule path_rule)
	{
		if (!path_rule) {
			return GetFallbackSection(normalized_path);
		}

		if (path_rule.SectionId != string.Empty) {
			return path_rule.SectionId;
		}

		string relative_path = normalized_path;
		if (MatchesPathPrefix(normalized_path, path_rule.Path) && normalized_path.Length() > path_rule.Path.Length()) {
			relative_path = normalized_path.Substring(path_rule.Path.Length(), normalized_path.Length() - path_rule.Path.Length());
		}

		while (relative_path.Length() > 0 && relative_path.Substring(0, 1) == "/") {
			relative_path = relative_path.Substring(1, relative_path.Length() - 1);
		}

		array<string> relative_segments = SplitPath(relative_path);
		if (relative_segments.Count() > 1) {
			return relative_segments[0];
		}

		return GetFallbackSection(normalized_path);
	}

	protected static string GetFallbackSection(string normalized_path)
	{
		array<string> segments = SplitPath(normalized_path);
		if (segments.Count() < 2) {
			return "unknown";
		}

		for (int i = segments.Count() - 2; i >= 0; i--) {
			if (!IsNoiseSegment(segments[i])) {
				return segments[i];
			}
		}

		return "unknown";
	}

	protected static array<string> SplitPath(string normalized_path)
	{
		ref array<string> raw_segments = {};
		ref array<string> normalized_segments = {};
		if (normalized_path == string.Empty) {
			return normalized_segments;
		}

		normalized_path.Split("/", raw_segments);
		foreach (string raw_segment: raw_segments) {
			string normalized_segment = NormalizeId(StripExtension(raw_segment));
			if (normalized_segment == string.Empty) {
				continue;
			}

			normalized_segments.Insert(normalized_segment);
		}

		return normalized_segments;
	}

	protected static bool IsNoiseSegment(string segment)
	{
		return NOISE_SEGMENTS.Find(segment) != -1;
	}

	protected static EditorBuildMenuMatchResult MatchRegisteredEntry(EditorPlaceableItem placeable, string normalized_path)
	{
		if (!placeable) {
			return null;
		}

		DayZEditorBuildMenuEntryRegistration best_registration;
		string type_lower = placeable.Type;
		type_lower.ToLower();

		foreach (DayZEditorBuildMenuEntryRegistration registration: s_RegisteredEntries) {
			if (!registration || !EditorBuildMenuTaxonomy.IsBuiltinSubcategoryId(NormalizeId(registration.TabId), NormalizeId(registration.SubcategoryId))) {
				continue;
			}

			if (!RegistrationMatches(placeable, registration, type_lower, normalized_path)) {
				continue;
			}

			if (!best_registration || PreferRegistration(registration, best_registration)) {
				best_registration = registration;
			}
		}

		if (!best_registration) {
			return null;
		}

		return CreateResult(best_registration.TabId, best_registration.SubcategoryId, best_registration.SectionId, GetSourceKind(placeable, normalized_path, string.Empty, best_registration.SourceKind, best_registration.SourceLabel), GetSourceId(placeable, normalized_path, string.Empty, best_registration.SourceKind, best_registration.SourceLabel), GetSourceLabel(placeable, normalized_path, string.Empty, best_registration.SourceKind, best_registration.SourceLabel));
	}

	protected static bool RegistrationMatches(EditorPlaceableItem placeable, DayZEditorBuildMenuEntryRegistration registration, string type_lower, string normalized_path)
	{
		if (!registration) {
			return false;
		}

		string match_value = registration.MatchValue;
		match_value.TrimInPlace();
		if (match_value == string.Empty) {
			return false;
		}

		switch (registration.MatchType) {
			case DayZEditorBuildMenuEntryMatchType.DAYZ_EDITOR_BUILD_MENU_MATCH_TYPE:
				match_value.ToLower();
				return type_lower == match_value;

			case DayZEditorBuildMenuEntryMatchType.DAYZ_EDITOR_BUILD_MENU_MATCH_MODEL_PREFIX:
				match_value = NormalizeTextPath(match_value);
				return MatchesPathPrefix(normalized_path, match_value);

			case DayZEditorBuildMenuEntryMatchType.DAYZ_EDITOR_BUILD_MENU_MATCH_ANCESTRY:
				return GetGame().IsKindOf(placeable.Type, match_value);
		}

		return false;
	}

	protected static bool PreferRegistration(DayZEditorBuildMenuEntryRegistration lhs, DayZEditorBuildMenuEntryRegistration rhs)
	{
		if (lhs.Priority != rhs.Priority) {
			return lhs.Priority < rhs.Priority;
		}

		return lhs.MatchType < rhs.MatchType;
	}

	protected static EditorBuildMenuMatchResult MatchScripted(EditorPlaceableItem placeable)
	{
		if (!placeable || placeable.Category != EditorPlaceableItemCategory.SCRIPTED) {
			return null;
		}

		string type_lower = placeable.Type;
		type_lower.ToLower();
		if (type_lower.Contains("particle")) {
			return CreateResult("effects", "particles", "particles", DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED, "dayzeditor", "DayZEditor");
		}

		if (type_lower.Contains("light")) {
			return CreateResult("effects", "lights", "lights", DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED, "dayzeditor", "DayZEditor");
		}

		return CreateResult("misc", "editor", "scripted", DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED, "dayzeditor", "DayZEditor");
	}

	protected static EditorBuildMenuMatchResult MatchActorOrEffect(EditorPlaceableItem placeable, string normalized_path)
	{
		if (!placeable) {
			return null;
		}

		string source_id = GetSourceId(placeable, normalized_path);
		string source_label = GetSourceLabel(placeable, normalized_path);
		int source_kind = GetSourceKind(placeable, normalized_path);
		string type_lower = placeable.Type;
		type_lower.ToLower();

		if (IsParticleType(placeable, type_lower)) {
			return CreateResult("effects", "particles", "particles", source_kind, source_id, source_label);
		}

		if (IsLightType(placeable, type_lower)) {
			return CreateResult("effects", "lights", "lights", source_kind, source_id, source_label);
		}

		if (GetGame().IsKindOf(placeable.Type, "ZombieBase")) {
			return CreateResult("creatures", "infected", "zombies", source_kind, source_id, source_label);
		}

		if (type_lower.IndexOf("animal_") == 0 || normalized_path.IndexOf("dz/animals/") == 0) {
			return CreateResult("creatures", "animals", GetAnimalSection(normalized_path, type_lower), source_kind, source_id, source_label);
		}

		if (GetGame().IsKindOf(placeable.Type, "Man")) {
			return CreateResult("creatures", "humans", "humans", source_kind, source_id, source_label);
		}

		return null;
	}

	protected static bool IsParticleType(EditorPlaceableItem placeable, string type_lower)
	{
		if (type_lower.Contains("particle")) {
			return true;
		}

		return GetGame().IsKindOf(placeable.Type, "NetworkParticleBase");
	}

	protected static bool IsLightType(EditorPlaceableItem placeable, string type_lower)
	{
		if (type_lower.Contains("light")) {
			return true;
		}

		if (GetGame().IsKindOf(placeable.Type, "DZ_LightAI")) {
			return true;
		}

		if (GetGame().IsKindOf(placeable.Type, "PointLightBase")) {
			return true;
		}

		return GetGame().IsKindOf(placeable.Type, "ScriptedLightBase");
	}

	protected static string GetAnimalSection(string normalized_path, string type_lower)
	{
		if (normalized_path.IndexOf("dz/animals/") == 0) {
			string relative_path = normalized_path.Substring("dz/animals/".Length(), normalized_path.Length() - "dz/animals/".Length());
			array<string> segments = SplitPath(relative_path);
			if (segments.Count() > 1) {
				return segments[0];
			}
		}

		if (type_lower.IndexOf("animal_") == 0) {
			array<string> parts = {};
			type_lower.Split("_", parts);
			if (parts.Count() > 1 && parts[1] != string.Empty) {
				return parts[1];
			}
		}

		return "animals";
	}

	protected static int GetSourceKind(EditorPlaceableItem placeable, string normalized_path, string explicit_source_id = string.Empty, int explicit_source_kind = -1, string explicit_source_label = string.Empty)
	{
		if (!placeable) {
			return DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED;
		}

		if (placeable.Category == EditorPlaceableItemCategory.SCRIPTED) {
			return DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED;
		}

		if (IsBuilderItemsPlaceable(placeable)) {
			return DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED;
		}

		string config_source_path = GetConfigSourcePath(placeable);
		if (ShouldUseConfigSourcePath(config_source_path, explicit_source_id, explicit_source_kind)) {
			return DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED;
		}

		if (explicit_source_id != string.Empty || explicit_source_label != string.Empty || explicit_source_kind != -1) {
			if (explicit_source_kind != -1) {
				return explicit_source_kind;
			}

			return DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED;
		}

		string source_path = GetSourcePath(placeable, normalized_path);
		if (IsVanillaSourcePath(source_path)) {
			return DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_VANILLA;
		}

		return DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_MODDED;
	}

	protected static string GetSourceId(EditorPlaceableItem placeable, string normalized_path, string explicit_source_id = string.Empty, int explicit_source_kind = -1, string explicit_source_label = string.Empty)
	{
		if (placeable && placeable.Category == EditorPlaceableItemCategory.SCRIPTED) {
			return "dayzeditor";
		}

		if (IsBuilderItemsPlaceable(placeable)) {
			return "builder_items";
		}

		string config_source_path = GetConfigSourcePath(placeable);
		if (ShouldUseConfigSourcePath(config_source_path, explicit_source_id, explicit_source_kind)) {
			string config_path_root = GetPathRoot(config_source_path);
			if (config_path_root != string.Empty) {
				return NormalizeId(config_path_root);
			}
		}

		if (explicit_source_id != string.Empty) {
			return NormalizeId(explicit_source_id);
		}

		if (explicit_source_kind == DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_VANILLA) {
			return "vanilla";
		}

		if (explicit_source_label != string.Empty) {
			return NormalizeId(explicit_source_label);
		}

		string source_path = GetSourcePath(placeable, normalized_path);
		if (IsVanillaSourcePath(source_path)) {
			return "vanilla";
		}

		string path_root = GetPathRoot(source_path);
		if (path_root != string.Empty) {
			return NormalizeId(path_root);
		}

		return "modded";
	}

	protected static string GetSourceLabel(EditorPlaceableItem placeable, string normalized_path, string explicit_source_id = string.Empty, int explicit_source_kind = -1, string explicit_source_label = string.Empty)
	{
		if (placeable && placeable.Category == EditorPlaceableItemCategory.SCRIPTED) {
			return "DayZEditor";
		}

		if (IsBuilderItemsPlaceable(placeable)) {
			return "Builder Items";
		}

		string config_source_path = GetConfigSourcePath(placeable);
		if (ShouldUseConfigSourcePath(config_source_path, explicit_source_id, explicit_source_kind)) {
			string config_path_root = GetPathRoot(config_source_path);
			if (config_path_root != string.Empty) {
				return FormatLabel(config_path_root);
			}
		}

		if (explicit_source_label != string.Empty) {
			return explicit_source_label;
		}

		if (explicit_source_id == "vanilla" || explicit_source_kind == DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_VANILLA) {
			return "Vanilla";
		}

		if (explicit_source_id != string.Empty) {
			return FormatLabel(explicit_source_id);
		}

		string source_path = GetSourcePath(placeable, normalized_path);
		if (IsVanillaSourcePath(source_path)) {
			return "Vanilla";
		}

		string path_root = GetPathRoot(source_path);
		if (path_root != string.Empty) {
			return FormatLabel(path_root);
		}

		return "Modded";
	}

	protected static bool IsBuilderItemsPlaceable(EditorPlaceableItem placeable)
	{
		if (!placeable) {
			return false;
		}

		string type_lower = placeable.Type;
		type_lower.ToLower();
		if (type_lower.Contains("bldr_")) {
			return true;
		}

		string path_lower = placeable.Path;
		path_lower.Replace("\\", "/");
		path_lower.ToLower();
		if (path_lower.Contains("builderitems/")) {
			return true;
		}

		string model_lower = placeable.GetModelName();
		model_lower.Replace("\\", "/");
		model_lower.ToLower();
		if (model_lower.Contains("builderitems/")) {
			return true;
		}

		return false;
	}

	protected static bool ShouldUseConfigSourcePath(string config_source_path, string explicit_source_id, int explicit_source_kind)
	{
		if (config_source_path == string.Empty) {
			return false;
		}

		if (explicit_source_id == string.Empty && explicit_source_kind == -1) {
			return true;
		}

		if (explicit_source_id == "vanilla") {
			return true;
		}

		return explicit_source_kind == DayZEditorBuildMenuSourceKind.DAYZ_EDITOR_BUILD_MENU_SOURCE_VANILLA;
	}

	protected static string GetSourcePath(EditorPlaceableItem placeable, string normalized_path)
	{
		string config_source_path = GetConfigSourcePath(placeable);
		if (config_source_path != string.Empty) {
			return config_source_path;
		}

		return normalized_path;
	}

	protected static string GetConfigSourcePath(EditorPlaceableItem placeable)
	{
		if (!placeable || placeable.Category != EditorPlaceableItemCategory.CONFIG) {
			return string.Empty;
		}

		string config_path = string.Format("%1 %2", placeable.Path, placeable.Type);
		string texture_path = GetConfigArraySourcePath(config_path + " hiddenSelectionsTextures");
		if (texture_path != string.Empty) {
			return texture_path;
		}

		string material_path = GetConfigArraySourcePath(config_path + " hiddenSelectionsMaterials");
		if (material_path != string.Empty) {
			return material_path;
		}

		return string.Empty;
	}

	protected static string GetConfigArraySourcePath(string config_path)
	{
		if (config_path == string.Empty || !GetGame().ConfigIsExisting(config_path)) {
			return string.Empty;
		}

		ref array<string> values = {};
		GetGame().ConfigGetTextArray(config_path, values);
		foreach (string value: values) {
			string normalized_value = NormalizeDedupePath(value);
			if (normalized_value == string.Empty) {
				continue;
			}

			if (normalized_value.IndexOf("/") == -1) {
				continue;
			}

			if (IsVanillaSourcePath(normalized_value)) {
				continue;
			}

			return normalized_value;
		}

		return string.Empty;
	}

	protected static bool IsVanillaSourcePath(string normalized_path)
	{
		if (normalized_path == string.Empty) {
			return false;
		}

		if (normalized_path.IndexOf("dz/") == 0) {
			return true;
		}

		return normalized_path.IndexOf("dz_") == 0;
	}

	protected static string GetPathRoot(string normalized_path)
	{
		array<string> segments = SplitPath(normalized_path);
		if (segments.Count() == 0) {
			return string.Empty;
		}

		return segments[0];
	}

	protected static EditorBuildMenuMatchResult CreateResult(string tab_id, string subcategory_id, string section_id, int source_kind, string source_id, string source_label)
	{
		EditorBuildMenuMatchResult result = new EditorBuildMenuMatchResult();
		result.TabId = NormalizeId(tab_id);
		result.SubcategoryId = NormalizeId(subcategory_id);
		if (!EditorBuildMenuTaxonomy.IsBuiltinSubcategoryId(result.TabId, result.SubcategoryId)) {
			result.TabId = "misc";
			result.SubcategoryId = "unknown";
		}

		result.SectionId = NormalizeId(section_id);
		if (result.SectionId == string.Empty) {
			result.SectionId = "unknown";
		}
		result.SourceId = NormalizeId(source_id);
		result.SourceLabel = source_label;
		result.Resolved = true;
		return result;
	}

	protected static string StripExtension(string value)
	{
		int extension_index = value.LastIndexOf(".");
		if (extension_index != -1) {
			return value.Substring(0, extension_index);
		}

		return value;
	}
}
