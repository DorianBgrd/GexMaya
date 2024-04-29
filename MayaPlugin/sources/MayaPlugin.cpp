#include <QApplication>
#include "MayaPlugin/include/MayaPlugin.h"

#include "maya/MQtUtil.h"
#include "maya/MSelectionList.h"
#include "maya/MFnPluginData.h"
#include "maya/MNodeMessage.h"
#include "maya/MArgList.h"
#include "maya/MFileIO.h"

#include "MayaPlugin/include/GraphView.h"
#include "MayaPlugin/include/Nodes.h"
#include "MayaPlugin/include/Types.h"

#include "MayaPlugin/include/Plugin.h"

#include "Gex/ui/include/PluginLoader.h"

#define GRAPH_ATTR "graph"
#define GRAPH_ATTR_SHORT "grh"


MTypeId GexMaya::GexNetworkNode::id = 0x7ffff;
MTypeId GexMaya::GraphData::id = 0x7ffff + 8;
MTypeId GexMaya::GraphAttributesData::id = 0x7ffff + 16;
MTypeId GexMaya::GexDeformer::id = 0x7ffff + 24;


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


GexMaya::GraphAttributesData::GraphAttributesData(): MPxData()
{

}


MTypeId GexMaya::GraphAttributesData::typeId() const
{
    return id;
}


MStatus GexMaya::GraphAttributesData::readASCII(const MArgList &argList, unsigned int &endOfTheLastParsedElement)
{
    MString str = argList.asString(endOfTheLastParsedElement++);

    MStringArray tokens;
    str.split('@', tokens);

    for (unsigned int i = 0; i < tokens.length(); i+=2)
    {

    }

    return MS::kSuccess;
}


MStatus GexMaya::GraphAttributesData::readBinary(std::istream &in, unsigned int length)
{
    return MS::kSuccess;
}


MStatus GexMaya::GraphAttributesData::writeASCII(std::ostream &out)
{
    MString allStrData;
    for (auto data : tuple)
    {
        if (!allStrData.isEmpty())
            allStrData += "@";

        MString strData;

        Gex::Attribute* attribute = data.first;
        MObjectHandle mayaAttribute = data.second;

        allStrData += MString(attribute->Longname().c_str()) + "@" +
                MFnAttribute(mayaAttribute.object()).name();
    }

    out << "\"" << allStrData << "\"";

    return MS::kSuccess;
}


MStatus GexMaya::GraphAttributesData::writeBinary(std::ostream &out)
{
    return writeASCII(out);
}


void GexMaya::GraphAttributesData::copy(const MPxData& src)
{
    if (src.typeId() == typeId())
    {
        auto other = static_cast<const GraphAttributesData*>(&src);
        tuple = other->tuple;
    }
}


MString GexMaya::GraphAttributesData::name() const
{
    return "Gex::ExtraAttrData";
}


void* GexMaya::GraphAttributesData::create()
{
    return new GraphAttributesData();
}


GexMaya::AttrTuple GexMaya::GraphAttributesData::Data() const
{
    return tuple;
}


void GexMaya::GraphAttributesData::SetData(AttrTuple data)
{
    tuple = data;
}


// -------------------------------------------------
// GEX CALLBACK ------------------------------------
// -------------------------------------------------

// -------------------------------------------------
// GEX NODE ----------------------------------------
// -------------------------------------------------
GexMaya::GexNode::GexNode(MPxNode* src, const MString& attr)
{
    mpxnode = src;
    profiler = Gex::MakeProfiler();
    graphAttributeName = attr;
}


void GexMaya::GexNode::AddCustomAttribute(Gex::Attribute* attribute)
{
    MObject mayaAttribute = MayaTypeRegistry::GetRegistry()->CreateAttribute(
            attribute, mpxnode->thisMObject());
    if (mayaAttribute.isNull())
        return;

    RegisterCustomAttribute(attribute, mayaAttribute);
}


void GexMaya::GexNode::RegisterCustomAttribute(
        Gex::Attribute* attribute, MObjectHandle mayaAttr)
{
    if (attribute->IsInput())
    {
        inputs.emplace_back(attribute, mayaAttr);
    }
    else
    {
        outputs.emplace_back(attribute, mayaAttr);
    }
}


void GexMaya::GexNode::RemoveCustomAttribute(Gex::Attribute*)
{

}


GexMaya::AttrTuple GexMaya::GexNode::Inputs() const
{
    return inputs;
}


GexMaya::AttrTuple GexMaya::GexNode::Outputs() const
{
    return outputs;
}


void GexMaya::GexNode::PushInputs(Gex::CompoundNode* graph, MDataBlock& dataBlock)
{
    MStatus success;
    MFnDependencyNode node(mpxnode->thisMObject());

    for (auto input : graph->InternalAttributes())
    {
        if (!input->IsInput())
            continue;

        MPlug mayaInput = node.findPlug(input->Name().c_str(), true);
        if (mayaInput.isNull())
        {
            continue;
        }

        MStatus inSuccess;
        auto inputData = dataBlock.inputValue(
                mayaInput.attribute(),
                &inSuccess);

        CHECK_MSTATUS(inSuccess);

        MayaTypeRegistry::GetRegistry()->ToAttribute(
                inputData, input);
    }
}


void GexMaya::GexNode::PullOutputs(Gex::CompoundNode* graph, MDataBlock& dataBlock)
{
    MStatus success;
    MFnDependencyNode node(mpxnode->thisMObject());

    for (auto output : graph->InternalAttributes())
    {
        if (!output->IsOutput())
            continue;

        MPlug mayaInput = node.findPlug(output->Name().c_str(), true);
        if (mayaInput.isNull())
        {
            continue;
        }

        MStatus inSuccess;
        auto outputData = dataBlock.outputValue(
                mayaInput.attribute(),
                &inSuccess);

        CHECK_MSTATUS(inSuccess);

        MayaTypeRegistry::GetRegistry()->ToAttribute(
                outputData, output);
    }
}


Gex::Profiler GexMaya::GexNode::GetProfiler() const
{
    return profiler;
}


Gex::Attribute* GexMaya::GexNode::ToGexAttr(MObject attr) const
{
    Gex::CompoundNode* graph = Graph();
    MFnAttribute mfnattr(attr);

    return graph->FindAttribute(mfnattr.name().asUTF8());
}


bool GexMaya::GexNode::IsGraphInput(MObject attr) const
{
    Gex::Attribute* gexAttr = ToGexAttr(attr);

    return (gexAttr && gexAttr->IsInput());
}


bool GexMaya::GexNode::IsGraphOutput(MObject attr) const
{
    Gex::Attribute* gexAttr = ToGexAttr(attr);

    return (gexAttr && gexAttr->IsOutput());
}


Gex::CompoundNode* GexMaya::GexNode::Graph() const
{
    MStatus success;
    MPlug plug = MFnDependencyNode(mpxnode->thisMObject()).findPlug(
            graphAttributeName, true, &success);

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
// GEX NETWORK NODE --------------------------------
// -------------------------------------------------
MObject GexMaya::GexNetworkNode::graphAttr;
MObject GexMaya::GexNetworkNode::extraAttr;


GexMaya::GexNetworkNode::GexNetworkNode(): GexNode(this)
{

}


void InitGraph(MObject node, const MObject& attr,
               const std::string& rootNodeType,
               const std::string& rootNodeName)
{
    MGlobal::displayInfo("Post constructor.");
    MFnDependencyNode depNode(node);
    auto plug = depNode.findPlug(attr, true);

    auto* graphData = new GexMaya::GraphData();

    Gex::Node* deformerGraph = Gex::NodeFactory::GetFactory()->CreateNode(
            rootNodeType, rootNodeName);

    graphData->SetGraph(Gex::CompoundNode::FromNode(deformerGraph));

    plug.setMPxData(graphData);
}


struct CompNodeWrapper
{
    GexMaya::GexNode* gexNode = nullptr;
    Gex::CompoundNode* compNode = nullptr;
};


void AttributeToPlugFunc(MNodeMessage::AttributeMessage msg,
                         MPlug &plug, void *clientData)
{
    auto* graph = ((CompNodeWrapper*) clientData)->compNode;
    GexMaya::GexNode* gexNode = ((CompNodeWrapper*) clientData)->gexNode;

    MString name = plug.name();
    std::string mname = name.asUTF8();
    if (msg == MNodeMessage::kAttributeAdded)
    {
        MFnAttribute attr(plug.attribute());
        Gex::Attribute* gexAttr = graph->FindAttribute(attr.name().asUTF8());
        if (gexAttr && gexAttr->IsUserDefined())
        {
            if (attr.isArray() && gexAttr->IsMulti())
            {
                gexNode->RegisterCustomAttribute(
                        gexAttr, MObjectHandle(plug.attribute()));
            }
        }
    }

    if (!MFileIO::isReadingFile())
    {
        MNodeMessage::removeCallback(MMessage::currentCallbackId());
    }
}


void GexMaya::GexNetworkNode::postConstructor()
{
    InitGraph(thisMObject(), graphAttr,
              "CompoundNode", "Graph");


    if (MFileIO::isReadingFile())
    {
        auto* wrapper = new CompNodeWrapper();
        wrapper->gexNode = this;
        wrapper->compNode = Graph();

        MObject this_ = thisMObject();
        MNodeMessage::addAttributeAddedOrRemovedCallback(
                this_, &AttributeToPlugFunc, wrapper);
    }
}




MStatus GexMaya::GexNetworkNode::initialize()
{
    MFnTypedAttribute at;
    graphAttr = at.create(GRAPH_ATTR, GRAPH_ATTR_SHORT, GraphData::id,
                          MFnPluginData().create(GraphData::id));
    at.setChannelBox(false);
    at.setKeyable(false);
    at.setHidden(true);

    addAttribute(graphAttr);

    return MS::kSuccess;
}


// If a depends on b.
MStatus GexMaya::GexNetworkNode::dependsOn(
        const MPlug& plug, const MPlug& other,
        bool &depends) const
{
    depends = (IsGraphOutput(plug.attribute()) &&
            IsGraphInput(other.attribute()));

    return MStatus::kSuccess;
}


MStatus GexMaya::GexNetworkNode::setDependentsDirty(const MPlug &plug,
                                                    MPlugArray &plugArray)
{
    if (!IsGraphInput(plug.attribute()))
    {
        return MS::kSuccess;
    }

    MFnDependencyNode n(thisMObject());
    for (auto a : Outputs())
    {
        MStatus status;
        auto at = n.findPlug(a.second.object(),
                             true, &status);
        if (!status)
        {
            continue;
        }

        plugArray.append(at);
    }

    return MS::kSuccess;
}


void* GexMaya::GexNetworkNode::creator()
{
    return new GexNetworkNode();
}


MStatus GexMaya::GexNetworkNode::compute(const MPlug &plug, MDataBlock &dataBlock)
{
    // Retrieve graph.
    MPxData* data = dataBlock.inputValue(graphAttr).asPluginData();
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
MObject GexMaya::GexDeformer::graphAttr;
MObject GexMaya::GexDeformer::testAttr;


GexMaya::GexDeformer::GexDeformer(): GexNode(this)
{

}


void GexMaya::GexDeformer::postConstructor()
{
    InitGraph(thisMObject(), graphAttr,
              "Deformer/Graph", "Graph");

    if (MFileIO::isReadingFile())
    {
        auto* wrapper = new CompNodeWrapper();
        wrapper->gexNode = this;
        wrapper->compNode = Graph();

        MObject this_ = thisMObject();
        MNodeMessage::addAttributeAddedOrRemovedCallback(
                this_, &AttributeToPlugFunc, wrapper);
    }
}


MStatus GexMaya::GexDeformer::deform(MDataBlock &block, MItGeometry &iter,
                                     const MMatrix &mat, unsigned int multiIndex)
{
    MStatus success = MS::kSuccess;
    MDataHandle graphHandle = block.inputValue(graphAttr, &success);

    CHECK_MSTATUS_AND_RETURN_IT(success)

    MPxData* graphData = graphHandle.asPluginData();
    if (graphData->typeId() != GraphData::id)
    {
        return MS::kFailure;
    }

    GraphData* cgdata = static_cast<GraphData*>(graphData);

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
        graph->Run(profiler);
    }

    PullOutputs(graph, block);
    return MS::kSuccess;
}


void* GexMaya::GexDeformer::create()
{
    return new GexDeformer();
}

MStatus GexMaya::GexDeformer::initialize()
{
    MFnTypedAttribute at;

    graphAttr = at.create(GRAPH_ATTR, GRAPH_ATTR_SHORT, GraphData::id);  //,
                          // defaultData.object());

    at.setChannelBox(false);
    at.setKeyable(false);
    at.setHidden(true);

    addAttribute(graphAttr);

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

        auto graphPlug = depnode.findPlug(GRAPH_ATTR, true);
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
                            gexnode->AddCustomAttribute(at);
                        });
            }

            if (argsdb.isFlagSet(UI_FLAG))
            {

                auto* ui = new GraphWindow(cgraph, MQtUtil::mainWindow());
                ui->show();

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
    if (!status)
    {
        status.perror("Failed registering data.");
        return status;
    }

    status = plugin.registerNode("GexNetwork",
                                 GexMaya::GexNetworkNode::id,
                                 &GexMaya::GexNetworkNode::creator,
                                 &GexMaya::GexNetworkNode::initialize
                                 );

    if (!status) {
        status.perror("Failed registering node type.");
        return status;
    }

    status = plugin.registerNode("GexDeformer",
                                 GexMaya::GexDeformer::id,
                                 &GexMaya::GexDeformer::create,
                                 &GexMaya::GexDeformer::initialize,
                                 MPxNode::kDeformerNode);

    if (!status) {
        status.perror("Failed registering node type.");
        return status;
    }

    status = plugin.registerCommand("GexNetworkGraph",
                                    &GexMaya::GexNetworkGraph::create);

    if (!status) {
        status.perror("Failed registering command.");
        return status;
    }

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
