#include <QApplication>
#include "MayaPlugin/include/MayaPlugin.h"

#include "maya/MQtUtil.h"
#include "maya/MSelectionList.h"
#include "maya/MFnPluginData.h"
#include "maya/MNodeMessage.h"
#include "maya/MArgList.h"
#include "maya/MFileIO.h"
#include "maya/MFnGenericAttribute.h"
#include "maya/MFnCompoundAttribute.h"
#include "maya/MFnTypedAttribute.h"

#include "MayaPlugin/include/GraphView.h"
#include "MayaPlugin/include/Nodes.h"
#include "MayaPlugin/include/Types.h"

#include "MayaPlugin/include/Plugin.h"

#include "Gex/ui/include/PluginLoader.h"

#define GEX_GRAPH_ATTR "gexGraph"
#define GEX_GRAPH_ATTR_SHORT "gxgrh"

#define GRAPH_INPUTS_ATTR "gexInputs"
#define GRAPH_INPUTS_ATTR_SHORT "gxi"

#define GRAPH_INPUTS_NAME_ATTR "gexInputName"
#define GRAPH_INPUTS_NAME_ATTR_SHORT "gxin"

#define GRAPH_INPUTS_VALUE_ATTR "gexInputValue"
#define GRAPH_INPUTS_VALUE_ATTR_SHORT "gxiv"

#define GRAPH_OUTPUTS_ATTR "gexOutputs"
#define GRAPH_OUTPUTS_ATTR_SHORT "gxo"

#define GRAPH_OUTPUTS_NAME_ATTR "gexOutputName"
#define GRAPH_OUTPUTS_NAME_ATTR_SHORT "gxon"

#define GRAPH_OUTPUTS_VALUE_ATTR "gexOutputValue"
#define GRAPH_OUTPUTS_VALUE_ATTR_SHORT "gxov"


MTypeId GexMaya::GexNetworkNode::id = 0x7ffff;
MTypeId GexMaya::GraphData::id = 0x7ffff + 8;
MTypeId GexMaya::GexDeformer::id = 0x7ffff + 16;


GexMaya::GraphData::GraphData(): MPxData()
{
    Gex::Node* n = Gex::NodeFactory::GetFactory()->CreateNode(
            "CompoundNode", "Graph");

    g.reset(Gex::CompoundNode::FromNode(n));
}


GexMaya::GraphData::~GraphData()
{

}


GexMaya::GraphPtr GexMaya::GraphData::SharedPtr() const
{
    return g;
}


Gex::CompoundNode* GexMaya::GraphData::Graph() const
{
    return g.get();
}


void GexMaya::GraphData::SetGraph(Gex::CompoundNode* node)
{
    g.reset(node);
}


MTypeId GexMaya::GraphData::typeId() const
{
    return id;
}


MStatus GexMaya::GraphData::readASCII(const MArgList &argList, unsigned int &endOfTheLastParsedElement)
{
    MString str = argList.asString(endOfTheLastParsedElement++);

    Gex::Feedback res;
    Gex::Node* loaded = Gex::LoadGraphString(str.asUTF8(), &res);
    if (!res)
    {
        MGlobal::displayError(res.message.c_str());
        return MS::kFailure;
    }

    g.reset(Gex::CompoundNode::FromNode(loaded));
    return MS::kSuccess;
}


MStatus GexMaya::GraphData::readBinary(std::istream &in, unsigned int length)
{
    return MS::kSuccess;
}

MStatus GexMaya::GraphData::writeASCII(std::ostream &out)
{
    std::string str;
    Gex::ExportToString(g.get(), str, false);

    out << quoted(str);
    return MS::kSuccess;
}

MStatus GexMaya::GraphData::writeBinary(std::ostream &out)
{
    return writeASCII(out);
}

void GexMaya::GraphData::copy(const MPxData &src)
{
    if (src.typeId() == id)
    {
        const GraphData* gdata = static_cast<const GraphData*>(&src);
        g = gdata->SharedPtr();
    }
}

MString GexMaya::GraphData::name() const
{
    return "Gex::GraphData";
}


void* GexMaya::GraphData::create()
{
    return new GraphData();
}


// -------------------------------------------------
// GEX NODE ----------------------------------------
// -------------------------------------------------


GexMaya::GexNode::GexNode(MPxNode* src)
{
    mpxnode = src;
    profiler = Gex::MakeProfiler();
}


MStatus GexMaya::GexNode::AddCustomAttribute(Gex::Attribute* attribute)
{
    // Find inputs plug and next entry index.
    MFnDependencyNode depnode(mpxnode->thisMObject());

    // Resolve node attribute names.
    bool isOutput = attribute->IsOutput();
    MString inputAttrNameStr = GRAPH_INPUTS_NAME_ATTR;
    MString inputAttrValueStr = GRAPH_INPUTS_VALUE_ATTR;
    if (isOutput)
    {
        inputAttrNameStr = GRAPH_OUTPUTS_NAME_ATTR;
        inputAttrValueStr = GRAPH_OUTPUTS_VALUE_ATTR;
    }

    // Resolve node attributes.
    MObject inputAttrName = depnode.attribute(inputAttrNameStr);
    MObject inputAttrValue = depnode.attribute(inputAttrValueStr);
    if (inputAttrValue.isNull() || inputAttrName.isNull())
    {
        MGlobal::displayError("No input attributes found.");
        return MS::kFailure;
    }

    MPlug inputNamePlug = depnode.findPlug(inputAttrName, true);
    MPlug inputValuePlug = depnode.findPlug(inputAttrValue, true);

    if (inputNamePlug.isNull() || inputValuePlug.isNull())
        return MS::kFailure;

    // Get new attribute index.
    MIntArray indices;
    inputNamePlug.getExistingArrayAttributeIndices(indices);

    int destIndex = -1;
    for (int index : indices)
    {
        if (index - destIndex > 1)
        {
            destIndex++;
            break;
        }

        destIndex = index;
    }

    destIndex++;
    // Create a "proxy" plug with the correct name.
    MObject attr = MayaTypeRegistry::GetRegistry()->CreateAttribute(
            attribute, mpxnode->thisMObject());

    if (attr.isNull())
    {
        MGlobal::displayError(("Could not create new attribute " + attribute->Name()).c_str());
        return MS::kFailure;
    }

    MPlug proxy = depnode.findPlug(attr, true);

    // Create the next index.
    MPlug newIndexName = inputNamePlug.elementByLogicalIndex(destIndex);
    newIndexName.setString(attribute->Name().c_str());

    MPlug newIndexValue = inputValuePlug.elementByLogicalIndex(destIndex);

    // Connect attribute to proxy.
    MDGModifier modifier;
    if (isOutput)
        modifier.connect(newIndexValue, proxy);
    else
        modifier.connect(proxy, newIndexValue);
    modifier.doIt();

    return MS::kSuccess;
}


void GexMaya::GexNode::RemoveCustomAttribute(Gex::Attribute*)
{

}


void GexMaya::GexNode::PushInputs(Gex::CompoundNode* graph, MDataBlock& dataBlock) {
    MStatus success;
    MFnDependencyNode node(mpxnode->thisMObject());

    MObject attributeName = node.attribute(GRAPH_INPUTS_NAME_ATTR);
    MObject attributeValue = node.attribute(GRAPH_INPUTS_VALUE_ATTR);

    if (attributeName.isNull() || attributeValue.isNull())
        return;

    MArrayDataHandle inputsNamesHandle = dataBlock.inputArrayValue(attributeName);
    MArrayDataHandle inputValuesHandle = dataBlock.inputArrayValue(attributeValue);

    if (!inputsNamesHandle.elementCount())
        return;

    do {
        MString attrName = inputsNamesHandle.inputValue().asString();
        Gex::Attribute *gexAttr = graph->FindAttribute(attrName.asUTF8());
        if (!gexAttr) {
            MGlobal::displayWarning("Gex attribute " + attrName + " not found.");
            continue;
        }

        MDataHandle indexValueHandle = inputValuesHandle.inputValue();
        MayaTypeRegistry::GetRegistry()->ToAttribute(
                indexValueHandle, gexAttr);
    }
    while (inputsNamesHandle.next());
}


void GexMaya::GexNode::PullOutputs(Gex::CompoundNode* graph, MDataBlock& dataBlock)
{
    MStatus success;
    MFnDependencyNode node(mpxnode->thisMObject());

    MObject attributeName = node.attribute(GRAPH_OUTPUTS_NAME_ATTR);
    MObject attributeValue = node.attribute(GRAPH_OUTPUTS_VALUE_ATTR);

    if (attributeName.isNull() || attributeValue.isNull())
        return;

    MArrayDataHandle outputsNamesHandle = dataBlock.outputArrayValue(attributeName);
    MArrayDataHandle outputValuesHandle = dataBlock.outputArrayValue(attributeValue);

    if (!outputsNamesHandle.elementCount())
        return;

    do {
        MString attrName = outputsNamesHandle.outputValue().asString();
        Gex::Attribute *gexAttr = graph->FindAttribute(attrName.asUTF8());
        if (!gexAttr) {
            MGlobal::displayWarning("Gex attribute " + attrName + " not found.");
            continue;
        }

        MDataHandle indexValueHandle = outputValuesHandle.outputValue();
        MayaTypeRegistry::GetRegistry()->ToMayaAttribute(
                gexAttr, indexValueHandle);
    }
    while (outputsNamesHandle.next());
}


Gex::Profiler GexMaya::GexNode::GetProfiler() const
{
    return profiler;
}


Gex::CompoundNode* GexMaya::GexNode::Graph() const
{
    MStatus success;
    MPlug plug = MFnDependencyNode(mpxnode->thisMObject()).findPlug(
            GEX_GRAPH_ATTR, true, &success);

    if (!success)
    {
        MGlobal::displayWarning("Could not find any "
                                "attached graph.");
        return nullptr;
    }

    auto mdata = plug.asMObject();

    MFnPluginData pdata(mdata);

    MPxData* data = pdata.data(&success);
    if (!success)
    {
        MGlobal::displayWarning("Could not retrieve plugin data.");
        return nullptr;
    }

    if (data->typeId() != GraphData::id)
    {
        MGlobal::displayWarning("Invalid plugin data type.");
        return nullptr;
    }

    auto gdata = static_cast<const GraphData*>(data);
    return gdata->Graph();
}


// -------------------------------------------------
// GRAPH METHODS -----------------------------------
// -------------------------------------------------
MStatus InitGraph(MObject node,
                  const std::string& rootNodeType,
                  const std::string& rootNodeName)
{
    MGlobal::displayInfo("Post constructor.");
    MFnDependencyNode depNode(node);
    MObject gexGraphAttr = depNode.attribute(GEX_GRAPH_ATTR);
    auto plug = depNode.findPlug(gexGraphAttr, true);

    if (plug.isNull())
    {
        MGlobal::displayError("Could not find graph plug.");
        return MS::kFailure;
    }

    MFnPluginData pluginData;
    pluginData.create(GexMaya::GraphData::id);

    MPxData* data = pluginData.data();
    auto* graphData = dynamic_cast<GexMaya::GraphData*>(data);
    if (!graphData)
    {
        return MS::kFailure;
    }

    Gex::Node* deformerGraph = Gex::NodeFactory::GetFactory()->CreateNode(
            rootNodeType, rootNodeName);

    graphData->SetGraph(Gex::CompoundNode::FromNode(deformerGraph));

    plug.setMObject(pluginData.object());
    return MS::kSuccess;
}

// -------------------------------------------------
// GEX NETWORK NODE --------------------------------
// -------------------------------------------------
GEX_NODE_ATTRIBUTES_DEF(GexMaya::GexNetworkNode)


GexMaya::GexNetworkNode::GexNetworkNode(): GexNode(this)
{

}



void GexMaya::GexNetworkNode::postConstructor()
{
    InitGraph(thisMObject(),
              "CompoundNode",
              "Graph");
}


MStatus GexMaya::GexNetworkNode::initialize()
{
    GEX_NODE_INITIALIZE()

    return MS::kSuccess;
}


void* GexMaya::GexNetworkNode::creator()
{
    return new GexNetworkNode();
}


MStatus GexMaya::GexNetworkNode::compute(const MPlug &plug, MDataBlock &dataBlock)
{
    // Retrieve graph.
    MPxData* data = dataBlock.inputValue(gexGraph).asPluginData();
    if (data->typeId() != GraphData::id)
    {
        return MS::kFailure;
    }

    Gex::CompoundNode* graph = static_cast<GraphData*>(data)->Graph();

    // Push maya data to graph inputs.
    PushInputs(graph, dataBlock);

    // Compute internal graph.
    auto prof = GetProfiler();
    bool result = graph->Run(prof);
    if (!result)
    {
        MGlobal::displayError("Failed evaluating Gex graph.");
        return MS::kFailure;
    }

    // Push out graph data to maya outputs.
    PullOutputs(graph, dataBlock);
    return MS::kSuccess;
}


// -------------------------------------------------
// DEFORMER ----------------------------------------
// -------------------------------------------------
GEX_NODE_ATTRIBUTES_DEF(GexMaya::GexDeformer)


GexMaya::GexDeformer::GexDeformer(): GexNode(this)
{

}


void GexMaya::GexDeformer::postConstructor()
{
    InitGraph(thisMObject(),
              "Deformer/Graph",
              "Graph");
}


MStatus GexMaya::GexDeformer::deform(MDataBlock &block, MItGeometry &iter,
                                     const MMatrix &mat, unsigned int multiIndex)
{
    MStatus success = MS::kSuccess;
    MDataHandle graphHandle = block.inputValue(gexGraph, &success);

    CHECK_MSTATUS_AND_RETURN_IT(success)

    MPxData* graphData = graphHandle.asPluginData();
    if (graphData->typeId() != GraphData::id)
    {
        return MS::kFailure;
    }

    auto* cgdata = static_cast<GraphData*>(graphData);

    Gex::CompoundNode* graph = cgdata->Graph();

    PushInputs(graph, block);

    auto* itAt = graph->GetAttribute("GeomIt");
    if (itAt)
    {
        MItGeometryWrapper wrapper(&iter);
        size_t w = typeid(wrapper).hash_code();

        bool preSet = itAt->Set<MItGeometryWrapper>(wrapper);
        if (!preSet)
        {
            MGlobal::displayError("Failed pre setting deformer graph.");
            return MS::kFailure;
        }

        auto profiler = Gex::MakeProfiler();
        if (!graph->Run(profiler))
        {
            MGlobal::displayError("Failed computing deformer graph.");
            return MS::kFailure;
        }
    }

//    PullOutputs(graph, block);
    return MS::kSuccess;
}


void* GexMaya::GexDeformer::create()
{
    return new GexDeformer();
}


MStatus GexMaya::GexDeformer::initialize()
{
    GEX_NODE_INITIALIZE()

//    attributeAffects(gexInputs, outputGeom);
    attributeAffects(gexInputName, outputGeom);
    attributeAffects(gexInputValue, outputGeom);
    attributeAffects(gexOutputName, outputGeom);

    return MS::kSuccess;
}


#define ADD_NODE_FLAG "-an"
#define ADD_NODE_FLAG_L "-addNode"

#define ADD_NODE_PARENT_FLAG "-p"
#define ADD_NODE_PARENT_FLAG_L "-parent"

#define CONNECT_FLAG "-c"
#define CONNECT_FLAG_L "-connect"

#define ADD_ATTRIBUTE_FLAG "-aa"
#define ADD_ATTRIBUTE_FLAG_L "-addAttribute"

#define UI_FLAG "-ui"
#define UI_FLAG_L "-userInterface"

#define NODE_LIST_FLAG "-nl"
#define NODE_LIST_FLAG_L "-nodeList"


#define GEX_NET_GRAPH_RETURN_STATUS(status) \
if (cgraph && removeCallback) \
{ \
    cgraph->DeregisterAttributeCallback(callback); \
}                                                 \
return status;


struct RemoveCallback
{
    Gex::CallbackId id = 0;
    Gex::CompoundNode* graph = nullptr;

    RemoveCallback(Gex::CallbackId cbid,
                   Gex::CompoundNode* nodeGraph)
    {
        id = cbid;
        graph = nodeGraph;
    }

    RemoveCallback(const RemoveCallback& other)
    {
        id = other.id;
        graph = other.graph;
    }

    void operator()() const
    {
        if (graph)
        {
            graph->DeregisterAttributeCallback(id);
        }
    }
};


MStatus GexMaya::GexNetworkGraph::doIt(const MArgList &args)
{
    MSyntax syntax;
    syntax.enableEdit(true);
    syntax.enableQuery(true);
    syntax.addFlag(ADD_NODE_FLAG, ADD_NODE_FLAG_L,
                   MSyntax::kString, MSyntax::kString);
    syntax.addFlag(ADD_NODE_PARENT_FLAG, ADD_NODE_PARENT_FLAG_L,
                   MSyntax::kString);
    syntax.addFlag(CONNECT_FLAG, CONNECT_FLAG_L, MSyntax::kString,
                   MSyntax::kString);
    syntax.addFlag(ADD_ATTRIBUTE_FLAG, ADD_ATTRIBUTE_FLAG_L,
                   MSyntax::kString, MSyntax::kString,
                   MSyntax::kLong);
    syntax.addFlag(UI_FLAG, UI_FLAG_L);
    syntax.addFlag(NODE_LIST_FLAG, NODE_LIST_FLAG_L);

    syntax.setObjectType(MSyntax::kStringObjects, 0, 1);

    MArgDatabase argsdb(syntax, args);

    MStringArray names;
    argsdb.getObjects(names);

    MDGModifier modifier;

    bool removeCallback = false;
    Gex::CallbackId callback = 0;
    Gex::CompoundNode* cgraph = nullptr;

    if (argsdb.isEdit() || argsdb.isQuery())
    {
        if (!names.length())
        {
            MGlobal::displayError("Edit mode needs a node name.");
            return MS::kFailure;
        }

        MObject node;
        MSelectionList list;
        list.add(names[0]);

        list.getDependNode(0, node);

        MFnDependencyNode depnode(node);

        auto graphPlug = depnode.findPlug(GEX_GRAPH_ATTR, true);
        if (graphPlug.isNull())
        {
            MGlobal::displayError("Provided node is not of the correct type.");
            return MS::kFailure;
        }

        MFnPluginData pldata(graphPlug.asMObject());
        MPxData* data = pldata.data();
        if (data->typeId() != GraphData::id)
        {
            MGlobal::displayError("Graph attribute is not of the correct type.");
            return MS::kFailure;
        }

        cgraph = static_cast<GraphData*>(data)->Graph();
        if (!cgraph)
        {
            MGlobal::displayError("Failed loading graph");
            return MS::kFailure;
        }

        MPxNode* userNode = depnode.userNode();
        if (argsdb.isEdit())
        {
            // Initialize callbacks on graph, to update the
            // graph when it changes.
            auto* gexnode = dynamic_cast<GexNode*>(userNode);

            if (gexnode)
            {
                removeCallback = true;
                callback = cgraph->RegisterAttributeCallback(
                        [gexnode](Gex::Attribute* at, const Gex::AttributeChange& c)
                        {
                            if (at->Node() != gexnode->Graph())
                                return;

                            if (c == Gex::AttributeChange::AttributeAdded)
                                gexnode->AddCustomAttribute(at);
                            else if (c == Gex::AttributeChange::AttributeRemoved)
                                gexnode->RemoveCustomAttribute(at);
                        });
            }
            else
            {
                GEX_NET_GRAPH_RETURN_STATUS(MS::kFailure)
            }

            if (argsdb.isFlagSet(UI_FLAG))
            {

                RemoveCallback rmcb(callback, cgraph);

                auto* ui = new GraphWindow(cgraph, rmcb, MQtUtil::mainWindow());
                ui->show();

                removeCallback = false;

                setResult(true);
            }

            else
            {
                if (argsdb.isFlagSet(ADD_NODE_FLAG))
                {
                    MString addnodetype = argsdb.flagArgumentString(ADD_NODE_FLAG, 0);
                    MString addnodename = argsdb.flagArgumentString(ADD_NODE_FLAG, 1);
                    if (!addnodetype.isEmpty())
                    {
                        if (!addnodename.isEmpty())
                        {
                            Gex::CompoundNode* p = cgraph;
                            if (argsdb.isFlagSet(ADD_NODE_PARENT_FLAG))
                            {
                                auto prt = argsdb.flagArgumentString(ADD_NODE_PARENT_FLAG,  0);
                                p = Gex::CompoundNode::FromNode(cgraph->GetNode(prt.asUTF8()));
                            }

                            if (!p)
                            {
                                MGlobal::displayError("Invalid parent specified");
                                GEX_NET_GRAPH_RETURN_STATUS(MS::kFailure);
                            }

                            auto* newNode = p->CreateNode(addnodetype.asUTF8(),
                                                               addnodename.asUTF8());
                            if (!newNode)
                            {
                                MGlobal::displayWarning("Failed creating node of type " +
                                                        addnodetype + " with name " + addnodename);
                                GEX_NET_GRAPH_RETURN_STATUS(MS::kFailure);
                            }

                            setResult(MString(newNode->Path().c_str()));
                        }
                    }

                    GEX_NET_GRAPH_RETURN_STATUS(MS::kSuccess)
                }

                if (argsdb.isFlagSet(ADD_ATTRIBUTE_FLAG))
                {
                    MString attrname = argsdb.flagArgumentString(ADD_ATTRIBUTE_FLAG, 0);
                    MString attrtype = argsdb.flagArgumentString(ADD_ATTRIBUTE_FLAG, 1);
                    long attrVtype = argsdb.flagArgumentInt(ADD_ATTRIBUTE_FLAG, 2);
                    if (attrname.isEmpty() || attrtype.isEmpty())
                    {
                        MGlobal::displayError("Attribute name and type must be specified.");
                        GEX_NET_GRAPH_RETURN_STATUS(MS::kFailure)
                    }

                    auto* attribute = cgraph->CreateAttributeFromTypeName(
                            attrname.asUTF8(), attrtype.asUTF8(), Gex::AttrValueType::Single,
                            Gex::AttrType(attrVtype));
                    if (!attribute)
                    {
                        MGlobal::displayError("Failed creating attribute "
                                              + attrname + " " + attrtype + " with type " + attrVtype);
                        GEX_NET_GRAPH_RETURN_STATUS(MS::kFailure)
                    }
                    attribute->SetInternal(true);
                    GEX_NET_GRAPH_RETURN_STATUS(MS::kSuccess)
                }

                if (argsdb.isFlagSet(CONNECT_FLAG))
                {
                    MString srcCnx = argsdb.flagArgumentString(CONNECT_FLAG, 0);
                    MString dstCnx = argsdb.flagArgumentString(CONNECT_FLAG, 1);
                    if (!srcCnx.isEmpty() && !dstCnx.isEmpty())
                    {
                        auto* src = cgraph->FindAttribute(srcCnx.asUTF8());
                        auto* dst = cgraph->FindAttribute(dstCnx.asUTF8());

                        if (!src)
                        {
                            MGlobal::displayError("Failed retrieving " + srcCnx + " attribute.");
                            GEX_NET_GRAPH_RETURN_STATUS(MS::kFailure);
                        }

                        if (!dst)
                        {
                            MGlobal::displayError("Failed retrieving " + dstCnx + " attribute.");
                            GEX_NET_GRAPH_RETURN_STATUS(MS::kFailure)
                        }

                        if (!dst->ConnectSource(src))
                        {
                            MGlobal::displayWarning(("Failed connecting attributes " +
                                                     src->Longname() + " with name " + dst->Longname()).c_str());
                            GEX_NET_GRAPH_RETURN_STATUS(MS::kFailure)
                        }
                    }
                }
            }

            // Finalize callbacks on graph.
            if (removeCallback)
            {
                cgraph->DeregisterAttributeCallback(callback);
            }

            GEX_NET_GRAPH_RETURN_STATUS(MS::kSuccess)
        }

        else if (argsdb.isQuery())
        {
            if (argsdb.isFlagSet(NODE_LIST_FLAG))
            {
                MStringArray nodes;

                for (const auto n : cgraph->GetNodes())
                {
                    nodes.append(n->Path().c_str());
                }

                setResult(nodes);
            }
        }
    }

    GEX_NET_GRAPH_RETURN_STATUS(MStatus::kSuccess)
}


bool GexMaya::GexNetworkGraph::isUndoable() const
{
    return false;
}


MString GexMaya::GexNetworkGraph::commandString() const
{
    return "GexNetworkGraph";
}


void* GexMaya::GexNetworkGraph::create()
{
    return new GexMaya::GexNetworkGraph();
}


MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "DB", "0.0", "Any");

    // Initialize ui plugin loader.
    Gex::Ui::UiPluginLoader::Initialize();

    // Load plugin.
    GexMaya::LoadPlugin();

    // Load ui plugin.
    GexMaya::LoadUiPlugin();

    status = plugin.registerData("Gex::GraphData", GexMaya::GraphData::id,
                                 &GexMaya::GraphData::create);
    CHECK_MSTATUS(status)

    status = plugin.registerNode("GexNetwork",
                                 GexMaya::GexNetworkNode::id,
                                 &GexMaya::GexNetworkNode::creator,
                                 &GexMaya::GexNetworkNode::initialize,
                                 MPxNode::kDependNode
                                 );

    CHECK_MSTATUS(status)

    status = plugin.registerNode("GexDeformer",
                                 GexMaya::GexDeformer::id,
                                 &GexMaya::GexDeformer::create,
                                 &GexMaya::GexDeformer::initialize,
                                 MPxNode::kDeformerNode);

    CHECK_MSTATUS(status)

    status = plugin.registerCommand("GexNetworkGraph",
                                    &GexMaya::GexNetworkGraph::create);

    CHECK_MSTATUS(status)

    return status;
}


MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);
    status = plugin.deregisterNode(GexMaya::GexNetworkNode::id);
    if (!status) {
        status.perror("Failed deregistering plugin.");
        return status;
    }

    status = plugin.deregisterCommand("GexNetworkGraph");

    if (!status) {
        status.perror("Failed registering plugin.");
        return status;
    }

    return status;
}
