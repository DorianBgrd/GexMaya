#include "MayaPlugin/include/Plugin.h"

#include "MayaPlugin/include/Types.h"
#include "MayaPlugin/include/UiTypes.h"
#include "MayaPlugin/include/Nodes.h"

#include "Gex/include/Gex.h"
#include "Tsys/include/tsys.h"
#include "UiTsys//include/uitsys.h"

#include "maya/MPoint.h"
#include "maya/MVector.h"
#include "maya/MMatrix.h"
#include "maya/MItGeometry.h"
#include "maya/MDataBlock.h"


void GexMaya::LoadPlugin()
{
    auto* registry = TSys::TypeRegistry::GetRegistry();

    registry->RegisterType<MPoint, GexMaya::MPointHandler>();
    registry->RegisterType<MVector, GexMaya::MVectorHandler>();
    registry->RegisterType<MMatrix, MMatrixHandler>();
    registry->RegisterType<MItGeometryWrapper, MItGeometryHandler>();
    registry->RegisterType<MDataBlock*, MDataBlockHandler>();

    auto* factory = Gex::NodeFactory::GetFactory();

    factory->RegisterNodeBuilder("MayaMath/MVectorAdd",
                                 new MVectorAddBuilder());
    factory->RegisterNodeBuilder("MayaMath/MPointAdd",
                                 new MPointAddBuilder());
    factory->RegisterNodeBuilder("MayaMath/MFloatVectorAdd",
                                 new MFloatVectorAddBuilder());;
    factory->RegisterNodeBuilder("MayaMath/MFloatPointAdd",
                                 new MFloatPointAddBuilder());
    factory->RegisterNodeBuilder("MayaMath/MVectorMult",
                                 new MVectorMultBuilder());
    factory->RegisterNodeBuilder("MayaMath/MPointMult",
                                 new MPointMultBuilder());
    factory->RegisterNodeBuilder("MayaMath/MFloatVectorMult",
                                 new MFloatVectorMultBuilder());
    factory->RegisterNodeBuilder("MayaMath/MFloatPointMult",
                                 new MFloatPointMultBuilder());

    factory->RegisterNodeBuilder("Deformer/GeomIter",
                                 new GeomIterBuilder());
    factory->RegisterNodeBuilder("Deformer/Graph",
                                 new DeformerGraphBuilder());
    factory->RegisterNodeBuilder("Deformer/GeomIter",
                                 new GeomIterBuilder());
}


void GexMaya::LoadUiPlugin()
{
    auto* engine = UiTSys::UiTypeEngine::GetEngine();

    engine->RegisterType(typeid(MVector).hash_code(), "MVector",
                         new MVectorInitWCreator(),
                         new MVectorWidgetCreator());

    engine->RegisterType(typeid(MPoint).hash_code(), "MPoint",
                         new MPointInitWCreator(),
                         new MPointWidgetCreator());

    engine->RegisterType(typeid(MFloatVector).hash_code(), "MFloatVector",
                                       new MFloatVectorInitWCreator(),
                                       new MFloatVectorWidgetCreator());

    engine->RegisterType(typeid(MFloatPoint).hash_code(), "MFloatPoint",
                                      new MFloatPointInitWCreator(),
                                      new MFloatPointWidgetCreator());
}
