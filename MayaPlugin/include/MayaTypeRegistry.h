#ifndef GEXMAYA_MAYATYPEREGISTRY_H
#define GEXMAYA_MAYATYPEREGISTRY_H

#include "api.h"

#include "maya/MGlobal.h"

#include <map>
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MDataHandle.h>
#include <typeinfo>

#include "Gex/include/Gex.h"


namespace GexMaya
{
    typedef size_t Hash;

    struct MayaAttrConverter
    {
        virtual MObject BuildMayaAttribute(Gex::Attribute* attr,
                                           MObject node) const = 0;

        virtual void ToAttribute(MDataHandle& maya, Gex::Attribute* attr) const = 0;

        virtual void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const = 0;
    };


    // Numeric attributes.
    struct NumericAttrConverter: public MayaAttrConverter
    {
        virtual MFnNumericData::Type NumericType() const = 0;

        MObject BuildMayaAttribute(Gex::Attribute* attr, MObject node) const override;
    };

    struct BoolAttrConverter: public NumericAttrConverter
    {
        MFnNumericData::Type NumericType() const override
        {
            return MFnNumericData::kBoolean;
        }

        void ToAttribute(MDataHandle& maya, Gex::Attribute* attr) const override
        {
            attr->Set(maya.asBool());
        }

        void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const override
        {
            maya.setBool(attr->Get<bool>());
        }
    };

    struct IntAttrConverter: public NumericAttrConverter
    {
        MFnNumericData::Type NumericType() const override
        {
            return MFnNumericData::kInt64;
        }

        void ToAttribute(MDataHandle& maya, Gex::Attribute* attr) const override
        {
            attr->Set(maya.asInt());
        }

        void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const override
        {
            Gex::Feedback success;
            maya.setInt(attr->Get<int>(&success));
            if (!success)
                MGlobal::displayInfo(("ERROR -> " + success.message).c_str());
        }
    };

    struct FloatAttrConverter: public NumericAttrConverter
    {
        MFnNumericData::Type NumericType() const override
        {
            return MFnNumericData::kFloat;
        }

        void ToAttribute(MDataHandle& maya, Gex::Attribute* attr) const override
        {
            attr->Set(maya.asFloat());
        }

        void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const override
        {
            MGlobal::displayInfo(("Set value " + std::to_string(attr->Get<float>())
                                  + " from " + attr->Name()).c_str());
            maya.setFloat(attr->Get<float>());
        }
    };

    struct DoubleAttrConverter: public NumericAttrConverter
    {
        MFnNumericData::Type NumericType() const override
        {
            return MFnNumericData::kDouble;
        }

        void ToAttribute(MDataHandle& maya, Gex::Attribute* attr) const override
        {
            attr->Set(maya.asDouble());
        }

        void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const override
        {
            maya.setDouble(attr->Get<double>());
        }
    };

    struct  StringAttrConverter: public MayaAttrConverter
    {
        MObject BuildMayaAttribute(Gex::Attribute* attr,
                                   MObject node) const override;

        void ToAttribute(MDataHandle& maya, Gex::Attribute* attr) const override
        {
            attr->Set(maya.asString());
        }

        void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const override
        {
            maya.set(attr->Get<std::string>().c_str());
        }
    };


    // Type registry.
    class GEX_MAYA MayaTypeRegistry
    {
        static std::map<Hash, MayaAttrConverter*> handles;
        static MayaTypeRegistry* instance;

        MayaTypeRegistry();
    public:
        bool RegisterAttrConverter(Hash hash, MayaAttrConverter* cvrt,
                                   bool force=false);

        template<typename T, typename Converter>
        bool RegisterAttrConverter()
        {
            return RegisterAttrConverter(typeid(T).hash_code(),
                                         new Converter());
        }

        MayaAttrConverter* GetConverter(Hash h) const;

        template<typename T>
        MayaAttrConverter* GetConverter() const
        {
            return GetConverter(typeid(T).hash_code());
        }

        MObject CreateAttribute(Gex::Attribute* attr,
                                MObject node) const;

        void ToAttribute(MDataHandle& maya, Gex::Attribute* attr) const;

        void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const;

        static MayaTypeRegistry* GetRegistry();
    };
}

#endif //GEXMAYA_MAYATYPEREGISTRY_H
