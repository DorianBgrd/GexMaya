#include "MayaPlugin/include/MayaTypeRegistry.h"
#include "maya/MFnDependencyNode.h"


MObject GexMaya::NumericAttrConverter::BuildMayaAttribute(
        Gex::Attribute* attr, MObject node) const
{
    MString name = attr->Name().c_str();

    return MFnNumericAttribute().create(
            name, name,
            NumericType());
}


MObject GexMaya::StringAttrConverter::BuildMayaAttribute(
        Gex::Attribute* attr, MObject node) const
{
    MString name = attr->Name().c_str();

    return MFnTypedAttribute().create(name, name, MFnData::kString);
}


std::map<GexMaya::Hash, GexMaya::MayaAttrConverter*> GexMaya::MayaTypeRegistry::handles;
GexMaya::MayaTypeRegistry* GexMaya::MayaTypeRegistry::instance = nullptr;


GexMaya::MayaTypeRegistry::MayaTypeRegistry()
{
    RegisterAttrConverter<bool, BoolAttrConverter>();
    RegisterAttrConverter<int, IntAttrConverter>();
    RegisterAttrConverter<float, FloatAttrConverter>();
    RegisterAttrConverter<double, DoubleAttrConverter>();
    RegisterAttrConverter<std::string, StringAttrConverter>();
}


bool GexMaya::MayaTypeRegistry::RegisterAttrConverter(
        Hash hash, MayaAttrConverter* cvrt,
        bool force)
{
    if (handles.count(hash) && !force)
    {
        return false;
    }

    handles[hash] = cvrt;
    return true;
}


GexMaya::MayaAttrConverter* GexMaya::MayaTypeRegistry::GetConverter(Hash h) const
{
    auto it = handles.find(h);
    if (it == handles.end())
        return nullptr;

    return it->second;
}


GexMaya::MayaTypeRegistry* GexMaya::MayaTypeRegistry::GetRegistry()
{
    if (!instance)
        instance = new MayaTypeRegistry();
    return instance;
}


MObject GexMaya::MayaTypeRegistry::CreateAttribute(
        Gex::Attribute* attr, MObject node) const
{
    auto* handle = GetConverter(attr->TypeHash());
    if (!handle)
    {
        return MObject::kNullObj;
    }

    MObject mayaAttr = handle->BuildMayaAttribute(attr, node);

    if (!mayaAttr.isNull())
    {
        auto mayaAt = MFnAttribute(node);
        mayaAt.setChannelBox(attr->IsInput());
        mayaAt.setKeyable(attr->IsInput());
        mayaAt.setStorable(attr->IsInput());

        MFnDependencyNode(node).addAttribute(mayaAttr);
    }

    return mayaAttr;
}


void GexMaya::MayaTypeRegistry::ToAttribute(MDataHandle& maya, Gex::Attribute* attr) const
{
    auto* handle = GetConverter(attr->TypeHash());
    if (!handle)
    {
        return;
    }

    handle->ToAttribute(maya, attr);
}


void GexMaya::MayaTypeRegistry::ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const
{
    auto* handle = GetConverter(attr->TypeHash());
    if (!handle)
    {
        MGlobal::displayError(("Found no handle for type " + attr->Name()).c_str());
        return;
    }

    handle->ToMayaAttribute(attr, maya);
}
