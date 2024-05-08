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
            attr->Set(maya.asGenericBool());
        }

        void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const override
        {
            maya.setGenericBool(attr->Get<bool>(), true);
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
            attr->Set(maya.asGenericInt());
        }

        void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const override
        {
            Gex::Feedback success;
            maya.setGenericInt(attr->Get<int>(&success), true);
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
            float v = maya.asGenericFloat();
            attr->Set(maya.asGenericFloat());
        }

        void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const override
        {
            double v = attr->Get<float>();
            maya.setGenericFloat(attr->Get<float>(), true);
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
            double v = maya.asGenericDouble();
            attr->Set(maya.asGenericDouble());
        }

        void ToMayaAttribute(Gex::Attribute* attr, MDataHandle& maya) const override
        {
            double v = attr->Get<double>();
            maya.setGenericDouble(attr->Get<double>(), true);
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
