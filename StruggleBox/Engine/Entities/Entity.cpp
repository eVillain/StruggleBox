#include "Entity.h"
#include "EntityComponent.h"
#include "GFXDefines.h"
#include "GFXHelpers.h"
#include "Color.h"

EntityID Entity::_nextEntityID = 1;

// This tells the compiler to create the following data type functions
// Using GetAttributeDataPtr<type> without defining the type here will not compile
void TemplateGuards(Entity* e)
{
    e->GetAttributeDataPtr<bool>("");
    e->GetAttributeDataPtr<int>("");
    e->GetAttributeDataPtr<unsigned int>("");
    e->GetAttributeDataPtr<std::string>("");
    e->GetAttributeDataPtr<float>("");
    e->GetAttributeDataPtr<double>("");
    e->GetAttributeDataPtr<Color>("");
    e->GetAttributeDataPtr<glm::vec2>("");
    e->GetAttributeDataPtr<glm::vec3>("");
    e->GetAttributeDataPtr<glm::vec4>("");
    e->GetAttributeDataPtr<glm::quat>("");
}

Entity::Entity(const std::string& entName)
{
    // Get entity ID and save in attributes
    m_ID = _nextEntityID++;
    this->AddAttribute<int>("ID");
    Attribute* idAttrib = (GetAttribute<int>("ID"));
    int& idA = idAttrib->as<int>();
    idA = m_ID;
    // Set default type and alignment, to be overridden after creation
    this->AddAttribute<int>("type");
    Attribute* typeAttrib = (GetAttribute<int>("type"));
    int& tA = typeAttrib->as<int>();
    tA = ENTITY_NONE;
    this->AddAttribute<int>("alignment");
    Attribute* alignmentAttrib = (GetAttribute<int>("alignment"));
    int& aA = alignmentAttrib->as<int>();
    aA = ALIGNMENT_NEUTRAL;
    // Add enttype as name attribute for now
    this->AddAttribute<std::string>("name");
    Attribute* nameAttrib = (GetAttribute<std::string>("name"));
    std::string& nA = nameAttrib->as<std::string>();
    nA = entName;
}

Entity::~Entity()
{
    ClearAttributes();
}

//Entity Interface
const unsigned int Entity::GetID() const
{
    return m_ID;
}

// Attribute Interface
bool Entity::HasAttribute(const std::string& attrName) const
{
    if ( m_Attributes.find( attrName ) != m_Attributes.end() ) { return true; }
    else { return false; }
}

void Entity::RemoveAttribute(const std::string& attrName)
{
    std::map<const std::string, Attribute*>::iterator it;
    it = m_Attributes.find( attrName );
    if ( it != m_Attributes.end() ) {
        m_Attributes.erase( it );
    }
}

void Entity::ClearAttributes()
{
    m_Attributes.clear();
}

template<typename T> void Entity::AddAttribute(const std::string& attrName)
{
    if ( m_Attributes.find( attrName ) == m_Attributes.end() ) {
        Attribute* newAttrib = new Attribute(T());
        m_Attributes[attrName] = (Attribute*)newAttrib;
    } else {
        printf("[Entity] attrib %s already added \n", attrName.c_str());
    }
}

template<typename T> void Entity::AddAttribute(const std::string& attrName,
                                               T value)
{
    if ( m_Attributes.find( attrName ) == m_Attributes.end() ) {
        Attribute* newAttrib = new Attribute(value);
        m_Attributes[attrName] = (Attribute*)newAttrib;
    } else {
        printf("[Entity] attrib %s already added \n", attrName.c_str());
    }
}

template<typename T> Attribute* Entity::GetAttribute(const std::string& attrName)
{
    if ( m_Attributes.find( attrName ) != m_Attributes.end() ) {
        return ((Attribute*)m_Attributes.at( attrName ));
        // Notify entity that attribute has been updated
        attributeUpdate = true;
    } else {
        return NULL;
    }
}

template<typename T> const Attribute* Entity::GetAttribute(const std::string& attrName) const
{
    if ( m_Attributes.find( attrName ) != m_Attributes.end() ) {
        return ((Attribute*) m_Attributes.at( attrName ));
    } else {
        return NULL;
    }
}

Attribute* Entity::GetAttributeBase(const std::string& attrName) const
{
    if ( m_Attributes.find( attrName ) != m_Attributes.end() ) {
        return m_Attributes.at( attrName );
    } else {
        return NULL;
    }
}

const std::vector<std::string> Entity::GetAttributeNames()
{
    std::map<const std::string, Attribute*>::iterator it;
    std::vector<std::string> names;
    for (it=m_Attributes.begin(); it != m_Attributes.end(); it++ )  {
        names.push_back(it->first);
    }
    return names;
}

const std::vector<std::string> Entity::GetAttributeTypes()
{
    std::map<const std::string, Attribute*>::iterator it;
    std::vector<std::string> typeNames;
    for (it=m_Attributes.begin(); it != m_Attributes.end(); it++ ) {
        std::string name = intToString(it->second->GetMagicNumber());
        typeNames.push_back(std::string(name));
    }
    return typeNames;
}

void Entity::Print()
{
    std::map<const std::string, Attribute*>::iterator it;
    std::vector<std::string> typeNames;
    for (it=m_Attributes.begin(); it != m_Attributes.end(); it++ ) {
        if ( it->second->IsType<bool>() ) {
            printf("Attribute<bool> %s: %i\n", it->first.c_str(), it->second->as<bool>());
        } else if ( it->second->IsType<int>() ) {
            printf("Attribute<int> %s: %i\n", it->first.c_str(), it->second->as<int>());
        } else if ( it->second->IsType<unsigned int>() ) {
            printf("Attribute<unsigned int> %s: %i\n", it->first.c_str(), it->second->as<unsigned int>());
        } else if ( it->second->IsType<float>() ) {
            printf("Attribute<float> %s: %f\n", it->first.c_str(), it->second->as<float>());
        } else if ( it->second->IsType<double>() ) {
            printf("Attribute<double> %s: %f\n", it->first.c_str(), it->second->as<double>());
        } else if ( it->second->IsType<std::string>() ) {
            printf("Attribute<std::string> %s: %s\n", it->first.c_str(), it->second->as<std::string>().c_str());
        } else if ( it->second->IsType<glm::vec2>() ) {
            printf("Attribute<glm::vec2> %s: %f,%f\n", it->first.c_str(),
                   it->second->as<glm::vec2>().x,it->second->as<glm::vec2>().y);
        } else if ( it->second->IsType<glm::vec3>() ) {
            printf("Attribute<glm::vec3> %s: %f,%f,%f\n", it->first.c_str(),
                   it->second->as<glm::vec3>().x,it->second->as<glm::vec3>().y,it->second->as<glm::vec3>().z);
        } else if ( it->second->IsType<glm::vec4>() ) {
            printf("Attribute<glm::vec4> %s: %f,%f,%f,%f\n", it->first.c_str(),
                   it->second->as<glm::vec4>().x,it->second->as<glm::vec4>().y,it->second->as<glm::vec4>().z,it->second->as<glm::vec4>().w);
        } else if ( it->second->IsType<glm::quat>() ) {
            printf("Attribute<glm::quat> %s: %f,%f,%f,%f\n", it->first.c_str(),
                   it->second->as<glm::quat>().x,it->second->as<glm::quat>().y,it->second->as<glm::quat>().z,it->second->as<glm::quat>().w);
        }
    }
}

