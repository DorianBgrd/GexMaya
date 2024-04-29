#include "MayaPlugin/include/Types.h"

#include "maya/MDataBlock.h"
#include "maya/MMatrix.h"
#include "maya/MWeight.h"


std::any GexMaya::MDataBlockHandler::CopyValue(std::any source) const
{
    MDataBlock* block = std::any_cast<MDataBlock*>(source);
    return std::make_any<MDataBlock*>(block);
}


size_t GexMaya::MDataBlockHandler::Hash() const
{
    return typeid(MDataBlock*).hash_code();
}


std::string GexMaya::MDataBlockHandler::ApiName() const
{
    return "MDataBlock";
}


std::string GexMaya::MDataBlockHandler::Name() const
{
    return ApiName();
}


std::string GexMaya::MDataBlockHandler::PythonName() const
{
    return ApiName();
}


size_t GexMaya::MDataBlockHandler::ValueHash(std::any val) const
{
    return 0;
}


std::any GexMaya::MItGeometryHandler::InitValue() const
{
    return std::make_any<MItGeometryWrapper>(MItGeometryWrapper());
}


std::any GexMaya::MItGeometryHandler::CopyValue(std::any source) const
{
    MItGeometryWrapper block = std::any_cast<MItGeometryWrapper>(source);
    return std::make_any<MItGeometryWrapper>(block);
}


size_t GexMaya::MItGeometryHandler::Hash() const
{
    return typeid(MItGeometryWrapper).hash_code();
}


std::string GexMaya::MItGeometryHandler::ApiName() const
{
    return "MItGeometry";
}


std::string GexMaya::MItGeometryHandler::Name() const
{
    return ApiName();
}


std::string GexMaya::MItGeometryHandler::PythonName() const
{
    return ApiName();
}



size_t GexMaya::MItGeometryHandler::ValueHash(std::any val) const
{
    return 0;
}


std::any GexMaya::MMatrixHandler::CopyValue(std::any source) const
{
    MMatrix m = std::any_cast<MMatrix>(source);
    return std::make_any<MMatrix>(m);
}


size_t GexMaya::MMatrixHandler::Hash() const
{
    return typeid(MMatrix).hash_code();
}


std::string GexMaya::MMatrixHandler::ApiName() const
{
    return "MMatrix";
}

std::string GexMaya::MMatrixHandler::Name() const
{
    return ApiName();
}


std::string GexMaya::MMatrixHandler::PythonName() const
{
    return ApiName();
}


size_t GexMaya::MMatrixHandler::ValueHash(std::any val) const
{
    return 0;
}