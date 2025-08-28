/**
 * @class UI_BindingComponent
 * @brief Holds a reference to the UI list item associated with an entity.
 *
 * This component links an ECS entity to its representation in the editor's UI.
 */
class UI_BindingComponent : EcsComponent
{
    EditorPlacedListItem ListItem;

    void UI_BindingComponent(EditorPlacedListItem item)
    {
        ListItem = item;
    }
}