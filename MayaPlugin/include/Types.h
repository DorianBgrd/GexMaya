#ifndef GEXMAYA_TYPES_H
#define GEXMAYA_TYPES_H

#include "rapidjson/document.h"
#include "Tsys/include/tsys.h"
#include "Tsys/include/defaultTypes.h"
#include "Gex/Gex/include/Gex.h"
#include "boost/python.hpp"

#include "maya/MVector.h"
#include "maya/MMatrix.h"
#include "maya/MItGeometry.h"

namespace GexMaya
{
    struct UnsavableHandler: public TSys::TypeHandler
    {
        // Disable saving options.
        void SerializeValue(std::any v, rapidjson::Value& value,
                            rapidjson::Document& document)
        const override {};

        // Disable loading options.
        std::any DeserializeValue(std::any v,
                                  rapidjson::Value& value)
        const override
        {
            return {};
        };


        void SerializeConstruction(std::any v, rapidjson::Value& value,
                                   rapidjson::Document& document)
        const override {};

        std::any DeserializeConstruction(
                rapidjson::Value& value)
        const override
        {
            return {};
        };
    };


    template <typename P>
    struct PointHandler: public UnsavableHandler
    {
        std::any InitValue() const override
        {
            return std::make_any<P>(P());
        }

        bool CompareValue(std::any v1, std::any v2) const override
        {
            return std::any_cast<P>(v1) == std::any_cast<P>(v2);
        }

        std::any FromPython(boost::python::object) const override
        {
            return {};
        }

        boost::python::object ToPython(std::any) const override
        {
            return {};
        }

        std::any CopyValue(std::any source) const override
        {
            return source;
        }

        size_t Hash() const override
        {
            return typeid(P).hash_code();
        }

        size_t ValueHash(std::any val) const override
        {
            return 0;
        }

        std::string Name() const override
        {
            return typeid(P).name();
        }
    };

    // -------------------------------------------------------------------------
    // Converters --------------------------------------------------------------
    // -------------------------------------------------------------------------
    template<typename From, typename To>
    struct SimpleConverter: public TSys::TypeConverter
    {
        std::any Convert(std::any from, std::any to) const override
        {
            return std::make_any<To>(std::any_cast<From>(from));
        }
    };


    // MFloatPoint conversion ---
    struct MFloatPointToMPoint: public SimpleConverter<MFloatPoint, MPoint> {};


    struct MFloatPointToMVector: public SimpleConverter<MFloatPoint, MVector> {};


    struct MFloatPointToMFloatVector: public SimpleConverter<MFloatPoint, MFloatVector> {};

    // MPoint conversion ---
    struct MPointToMFloatPoint: public SimpleConverter<MPoint, MFloatPoint> {};


    struct MPointToMVector: public SimpleConverter<MPoint, MVector> {};


    struct MPointToMFloatVector: public SimpleConverter<MPoint, MFloatVector> {};


    // MVector conversion ---
    struct MVectorToMPoint: public SimpleConverter<MVector, MPoint> {};


    struct MVectorToMFloatPoint: public SimpleConverter<MVector, MFloatPoint> {};


    struct MVectorToMFloatVector: public SimpleConverter<MVector, MFloatVector> {};


    // MFloatVector conversion ---
    struct MFloatVectorToMPoint: public SimpleConverter<MFloatVector, MPoint> {};


    struct MFloatVectorToMFloatPoint: public SimpleConverter<MFloatVector, MFloatPoint> {};


    struct MFloatVectorToMVector: public SimpleConverter<MFloatVector, MVector> {};


    // -------------------------------------------------------------------------
    // Handlers ----------------------------------------------------------------
    // -------------------------------------------------------------------------
    struct MVectorHandler: public PointHandler<MVector>
    {
        MVectorHandler()
        {
            RegisterConverter<MPoint, MPointToMVector>();
            RegisterConverter<MFloatPoint, MFloatPointToMVector>();
            RegisterConverter<MFloatVector, MFloatVectorToMVector>();
        }

        std::string PythonName() const override
        {
            return "MVector";
        }

        std::string ApiName() const override
        {
            return "MVector";
        }
    };


    struct MFloatVectorHandler: public PointHandler<MFloatVector>
    {
        MFloatVectorHandler()
        {
            RegisterConverter<MVector, MVectorToMFloatVector>();
            RegisterConverter<MPoint, MPointToMFloatVector>();
            RegisterConverter<MFloatPoint, MFloatPointToMFloatVector>();
        }

        std::string PythonName() const override
        {
            return "MFloatVector";
        }

        std::string ApiName() const override
        {
            return "MFloatVector";
        }
    };


    struct MPointHandler: public PointHandler<MPoint>
    {
        MPointHandler()
        {
            RegisterConverter<MFloatPoint, MFloatPointToMPoint>();
            RegisterConverter<MVector, MVectorToMPoint>();
            RegisterConverter<MFloatVector, MFloatVectorToMPoint>();
        }

        std::string PythonName() const override
        {
            return "MPoint";
        }

        std::string ApiName() const override
        {
            return "MPoint";
        }
    };


    struct MFloatPointHandler: public PointHandler<MFloatPoint>
    {
        MFloatPointHandler()
        {
            RegisterConverter<MPoint, MPointToMFloatPoint>();
            RegisterConverter<MVector, MVectorToMFloatPoint>();
            RegisterConverter<MFloatVector, MFloatVectorToMFloatPoint>();
        }

        std::string PythonName() const override
        {
            return "MFloatPoint";
        }

        std::string ApiName() const override
        {
            return "MFloatPoint";
        }
    };



    struct ReadOnlyHandler: public UnsavableHandler
    {
        std::any InitValue() const override
        {
            return {};
        }

        bool CompareValue(std::any, std::any) const override
        {
            return false;
        }

        std::any FromPython(boost::python::object) const override
        {
            return {};
        }

        boost::python::object ToPython(std::any) const override
        {
            return {};
        }
    };


    struct MDataBlockHandler: public ReadOnlyHandler
    {
        std::any CopyValue(std::any source) const override;

        size_t Hash() const override;

        std::string Name() const override;

        std::string PythonName() const override;

        std::string ApiName() const override;

        size_t ValueHash(std::any val) const override;
    };


    struct MItGeometryHandler: public ReadOnlyHandler
    {
        std::any CopyValue(std::any source) const override;

        size_t Hash() const override;

        std::string Name() const override;

        std::string PythonName() const override;

        std::string ApiName() const override;

        size_t ValueHash(std::any val) const override;
    };


    struct MMatrixHandler: public ReadOnlyHandler
    {
        std::any CopyValue(std::any source) const override;

        size_t Hash() const override;

        std::string Name() const override;

        std::string PythonName() const override;

        std::string ApiName() const override;

        size_t ValueHash(std::any val) const override;
    };

}

#endif //GEXMAYA_TYPES_H
