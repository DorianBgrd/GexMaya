#ifndef GEXMAYA_MAYAPLUGIN_H
#define GEXMAYA_MAYAPLUGIN_H

#include "MayaTypeRegistry.h"

#include "Gex/Gex/include/Gex.h"
#include "Gex/ui/include/GraphView.h"

#include "maya/MGlobal.h"
#include "maya/MStatus.h"
#include "maya/MPxNode.h"
#include "maya/MPxData.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnPlugin.h"
#include "maya/MPxCommand.h"
#include "maya/MSyntax.h"
#include "maya/MArgDatabase.h"
#include "maya/MDGModifier.h"
#include "maya/MObjectHandle.h"
#include "maya/MPxDeformerNode.h"
#include "maya/MNodeMessage.h"

#include "api.h"
#include "defs.h"

#include <map>
#include <vector>


namespace GexMaya
{
    struct GEX_MAYA GexNode
    {
    private:
        Gex::Profiler profiler;
        MPxNode* mpxnode;

    public:
        GexNode(MPxNode* node);

        Gex::CompoundNode* Graph() const;

        MStatus AddCustomAttribute(Gex::Attribute*);

        void RemoveCustomAttribute(Gex::Attribute*);

        void PushInputs(Gex::CompoundNode* graph,
                        MDataBlock& dataBlock);

        void PullOutputs(Gex::CompoundNode* graph,
                         MDataBlock& dataBlock);

        Gex::Profiler GetProfiler() const;
    };


    class GEX_MAYA GexNetworkNode: public MPxNode, public GexNode
    {
    public:
        static MTypeId id;

        GEX_NODE_ATTRIBUTES

        GexNetworkNode();

        void postConstructor() override;

        MStatus compute(const MPlug &plug, MDataBlock &dataBlock) override;

        static void *creator();

        static MStatus initialize();
    };


    class GEX_MAYA GexDeformer: public MPxDeformerNode, public GexNode
    {
    public:
        static MTypeId id;

        GEX_NODE_ATTRIBUTES

        GexDeformer();

        void postConstructor() override;

        MStatus deform(MDataBlock &block, MItGeometry &iter,
                       const MMatrix &mat, unsigned int multiIndex);

        static void* create();

        static MStatus initialize();
    };


    class GEX_MAYA GexNetworkGraph: public MPxCommand
    {
    public:
        MStatus doIt(const MArgList &args) override;

        bool isUndoable () const override;

        MString commandString()	const;

        static void* create();
    };
}

PLUGIN_EXPORT
MStatus initializePlugin(MObject obj);

PLUGIN_EXPORT
MStatus uninitializePlugin(MObject obj);


#endif //GEXMAYA_MAYAPLUGIN_H
