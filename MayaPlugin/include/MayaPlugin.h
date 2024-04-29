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

#include <vector>


namespace GexMaya
{
    typedef std::shared_ptr<Gex::CompoundNode> GraphPtr;
    typedef std::vector<std::pair<Gex::Attribute*, MObjectHandle>> AttrTuple;

    class GEX_MAYA GraphData: public MPxData
    {
        GraphPtr g;

    public:
        GraphData();

        ~GraphData();

        GraphPtr SharedPtr() const;

        Gex::CompoundNode* Graph() const;

        void SetGraph(Gex::CompoundNode* node);

        static MTypeId id;

        MTypeId typeId() const override;

        MStatus readASCII(const MArgList &argList, unsigned int &endOfTheLastParsedElement) override;

        MStatus readBinary(std::istream &in, unsigned int length) override;

        MStatus writeASCII(std::ostream &out) override;

        MStatus writeBinary(std::ostream &out) override;

        void copy(const MPxData& src) override;

        MString name() const override;

        static void* create();
    };


    class GEX_MAYA GraphAttributesData: public MPxData
    {
        AttrTuple tuple;

    public:
        GraphAttributesData();

        static MTypeId id;

        MTypeId typeId() const override;

        MStatus readASCII(const MArgList &argList, unsigned int &endOfTheLastParsedElement) override;

        MStatus readBinary(std::istream &in, unsigned int length) override;

        MStatus writeASCII(std::ostream &out) override;

        MStatus writeBinary(std::ostream &out) override;

        void copy(const MPxData& src) override;

        MString name() const override;

        static void* create();

        AttrTuple Data() const;

        void SetData(AttrTuple data);
    };


    struct GEX_MAYA GexNode
    {
    private:
        AttrTuple inputs;
        AttrTuple outputs;
        MPxNode* mpxnode;
        MString graphAttributeName;

        Gex::Profiler profiler;

    public:
        GexNode(MPxNode* node, const MString& graphAttributeName="graph");

        Gex::CompoundNode* Graph() const;

        void AddCustomAttribute(Gex::Attribute*);

        void RegisterCustomAttribute(Gex::Attribute* attribute,
                                     MObjectHandle mayaAttr);

        void RemoveCustomAttribute(Gex::Attribute*);

        AttrTuple Inputs() const;

        AttrTuple Outputs() const;

        Gex::Attribute* ToGexAttr(MObject attr) const;

        bool IsGraphInput(MObject attr) const;

        bool IsGraphOutput(MObject attr) const;

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

        static MObject graphAttr;

        static MObject extraAttr;

        GexNetworkNode();

        void postConstructor() override;

        MStatus compute(const MPlug &plug, MDataBlock &dataBlock) override;

        static void *creator();

        static MStatus initialize();

        MStatus dependsOn(const MPlug& plug, const MPlug& other,
                          bool &depends) const override;

        MStatus setDependentsDirty(const MPlug &plug,
                                   MPlugArray &plugArray)
                                   override;
    };


    class GEX_MAYA GexDeformer: public MPxDeformerNode, public GexNode
    {
    public:
        static MObject graphAttr;
        static MObject testAttr;
        static MTypeId id;

        GexDeformer();

        void postConstructor() override;

        MStatus deform(MDataBlock &block, MItGeometry &iter,
                       const MMatrix &mat, unsigned int multiIndex);

        static void* create();

        static MStatus initialize();
    };


    class GexNetworkGraph: public MPxCommand
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
