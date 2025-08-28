/**
 * @class SelectionComponent
 * @brief A tag component indicating that an entity is currently selected.
 *
 * Tag components are empty; their presence on an entity is what matters.
 * A SelectionSystem will add/remove this component to manage selection state.
 */
class SelectionComponent : EcsComponent
{
    // This component intentionally has no data.
}