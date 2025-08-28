/**
 * @class IdentifierComponent
 * @brief Holds identifying information for an entity, like its original class name.
 */
class IdentifierComponent : EcsComponent
{
    string ClassName;

    void IdentifierComponent(string name)
    {
        ClassName = name;
    }
}