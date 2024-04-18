#include <QApplication>
#include "MayaPlugin/include/MayaPlugin.h"

#include "maya/MQtUtil.h"
#include "maya/MSelectionList.h"
#include "maya/MFnPluginData.h"
#include "maya/MNodeMessage.h"
#include "maya/MArgList.h"

#include "MayaPlugin/include/GraphView.h"
#include "MayaPlugin/include/Nodes.h"

#include "MayaPlugin/include/Plugin.h"


MTypeId GexMaya::GexNetworkNode::id = 0x7ffff;
MTypeId GexMaya::GraphData::id = 0x7ffff + 8;
MTypeId GexMaya::ExtraAttrData::id = 0x7ffff + 16;
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


GexMaya::ExtraAttrData::ExtraAttrData(): MPxData()
{

}


MTypeId GexMaya::ExtraAttrData::typeId() const
{
    return id;
}


MStatus GexMaya::ExtraAttrData::readASCII(const MArgList &argList, unsigned int &endOfTheLastParsedElement)
{
    return MS::kSuccess;
}


MStatus GexMaya::ExtraAttrData::readBinary(std::istream &in, unsigned int length)
{
    return MS::kSuccess;
}


MStatus GexMaya::ExtraAttrData::writeASCII(std::ostream &out)
{
    return MS::kSuccess;
}


MStatus GexMaya::ExtraAttrData::writeBinary(std::ostream &out)
{
    return MS::kSuccess;
}


void GexMaya::ExtraAttrData::copy(const MPxData& src)
{
    if (src.typeId() == typeId())
    {
        auto other = static_cast<const ExtraAttrData*>(&src);
        tuple = other->tuple;
    }
}


MString GexMaya::ExtraAttrData::name() const
{
    return "Gex::ExtraAttrData";
}


void* GexMaya::ExtraAttrData::create()
{
    return new ExtraAttrData();
}


// -------------------------------------------------
// GEX NODE ----------------------------------------
// -------------------------------------------------
GexMaya::GexNode::GexNode(MPxNode* src)
{
    mpxnode = src;
}


void GexMaya::GexNode::AddCustomAttribute(Gex::Attribute* at)
{
    MObject mayaAttribute = MayaTypeRegistry::GetRegistry()->CreateAttribute(
            at, mpxnode->thisMObject());
    if (mayaAttribute.isNull())
        return;

    if (at->IsInput())
    {
        inputs.emplace_back(at, MObjectHandle(mayaAttribute));
    }
    else
    {
        outputs.emplace_back(at, MObjectHandle(mayaAttribute));
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


// -------------------------------------------------
// GEX NETWORK NODE --------------------------------
// -------------------------------------------------
MObject GexMaya::GexNetworkNode::graphAttr;
MObject GexMaya::GexNetworkNode::extraAttr;


void AttributeChangeCallback(GexMaya::GexNetworkNode* node, Gex::Attribute* attribute,
                             const Gex::AttributeChange& change)
{

}


GexMaya::GexNetworkNode::GexNetworkNode(): GexNode(this)
{
//    auto* cmp = Gex::NodeFactory::GetFactory()->CreateNode("CompoundNode", "Graph");
//    graph = Gex::CompoundNode::FromNode(cmp);
}


GexMaya::GexNetworkNode::~GexNetworkNode()
{
//    delete graph;
}


void GexMaya::GexNetworkNode::postConstructor()
{
    MFnDependencyNode node(thisMObject());

    MGlobal::displayInfo("postConstructor()");
    for (int i = 0; i < node.attributeCount(); i++)
    {
        auto attr = node.attribute(i);
        auto plug = node.findPlug(attr, true).name();

        MGlobal::displayInfo(" > " + plug);
    }
}


Gex::CompoundNode* GexMaya::GexNetworkNode::Graph() const
{
    MStatus success;
    MPlug plug = MFnDependencyNode(thisMObject()).findPlug(
            graphAttr, true, &success);

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


MStatus GexMaya::GexNetworkNode::initialize()
{
    MFnTypedAttribute at;
    graphAttr = at.create("graph", "sg", GraphData::id,
                          MFnPluginData().create(GraphData::id));
    at.setChannelBox(false);
    at.setKeyable(false);
    at.setHidden(true);

    addAttribute(graphAttr);

    return MS::kSuccess;
}


bool GexMaya::GexNetworkNode::IsGraphInput(MObject attr) const
{
    for (auto at : Inputs())
    {
        if (at.second.object() == attr)
        {
            return true;
        }
    }

    return false;
}


bool GexMaya::GexNetworkNode::IsGraphOutput(MObject attr) const
{
    for (auto at : Outputs())
    {
        if (at.second.object() == attr)
        {
            return true;
        }
    }

    return false;
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
    for (const auto& inp : Inputs())
    {
        MObjectHandle in = inp.second;
        if (!in.isValid())
        {
            MGlobal::displayWarning(("Skipping input " + inp.first->Longname()).c_str());
            continue;
        }

        MStatus inSuccess;
        auto inputData = dataBlock.inputValue(
                in.object(), &inSuccess);

        CHECK_MSTATUS(inSuccess);

        MayaTypeRegistry::GetRegistry()->ToAttribute(inputData, inp.first);
    }

    Gex::Profiler profiler = Gex::MakeProfiler();

    MPxData* data = dataBlock.inputValue(graphAttr).asPluginData();
    if (data->typeId() != GraphData::id)
    {
        return MS::kFailure;
    }

    Gex::CompoundNode* graph = static_cast<GraphData*>(data)->Graph();

    bool result = graph->Run(profiler);
    if (!result)
    {
        MGlobal::displayError("Failed evaluating Gex graph.");
        return MS::kFailure;
    }
    for (const auto& outp : Outputs())
    {
        MObjectHandle out = outp.second;
        if (!out.isValid())
        {
            MGlobal::displayWarning(("Skipping output " + outp.first->Longname()).c_str());
            continue;
        }

        MStatus outSuccess;
        auto outputData = dataBlock.outputValue(
                out.object(), &outSuccess);

        CHECK_MSTATUS(outSuccess);

        MayaTypeRegistry::GetRegistry()->ToMayaAttribute(outp.first, outputData);
        outputData.setClean();
    }

    return MS::kSuccess;
}




// -------------------------------------------------
// DEFORMER ----------------------------------------
// -------------------------------------------------
MObject GexMaya::GexDeformer::graphAttr;


GexMaya::GexDeformer::GexDeformer(): GexNode(this)
{

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

    auto* itAt = graph->GetAttribute("GeomIt");
    if (itAt)
    {
        itAt->Set(&iter);

        auto profiler = Gex::MakeProfiler();
        graph->Run(profiler);
    }

    return MS::kSuccess;
}


Gex::CompoundNode* GexMaya::GexDeformer::Graph() const
{
    MStatus success;
    MPlug plug = MFnDependencyNode(thisMObject()).findPlug(
            graphAttr, true, &success);

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


void* GexMaya::GexDeformer::create()
{
    return new GexDeformer();
}

MStatus GexMaya::GexDeformer::initialize()
{
    MFnTypedAttribute at;

    MObject deformerGraphObject = MFnPluginData().create(GraphData::id);

    MFnPluginData defaultData(deformerGraphObject);
    MPxData* data = defaultData.data();

    GexMaya::GraphData* defaultGraph = static_cast<GraphData*>(data);

    Gex::Node* deformerGraph = Gex::NodeFactory::GetFactory()
            ->CreateNode("Deformer/Graph", "Graph");

    defaultGraph->SetGraph(Gex::CompoundNode::FromNode(deformerGraph));

    graphAttr = at.create("graph", "sg", GraphData::id, defaultData.object());
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

//        if (depnode.typeId() != GexNetworkNode::id)
//        {
//            MGlobal::displayError("Provided node is not of the correct type.");
//            return MS::kFailure;
//        }

//        MPxNode* mpxnode = depnode.userNode();
//        auto* networkNode = static_cast<GexMaya::GexNetworkNode*>(mpxnode);
//        if (!networkNode)
//        {
//            return MS::kFailure;
//        }
//        auto* cgraph = networkNode->Graph();
        auto graphPlug = depnode.findPlug("graph", true);
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

        Gex::CompoundNode* cgraph = static_cast<GraphData*>(data)->Graph();
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

            bool removeCallback = false;
            Gex::CallbackId callback;
            if (gexnode)
            {
                removeCallback = true;
                callback = cgraph->RegisterAttributeCallback(
                        [gexnode](Gex::Attribute* at, const Gex::AttributeChange& c)
                        {
                            gexnode->AddCustomAttribute(at);
                        });
            }
//            auto cbFunc = [userNode](Gex::Attribute* attribute, const Gex::AttributeChange& change)
//            {
//                AttributeChangedCallback<>(userNode, attribute, change);
//            };
//
//            Gex::CallbackId callback = cgraph->RegisterAttributeCallback(cbFunc);

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
                                return MS::kFailure;
                            }

                            auto* newNode = p->CreateNode(addnodetype.asUTF8(),
                                                               addnodename.asUTF8());
                            if (!newNode)
                            {
                                MGlobal::displayWarning("Failed creating node of type " +
                                                        addnodetype + " with name " + addnodename);
                                return MS::kFailure;
                            }

                            setResult(MString(newNode->Path().c_str()));
                        }
                    }
                }

                if (argsdb.isFlagSet(ADD_ATTRIBUTE_FLAG))
                {
                    MString attrname = argsdb.flagArgumentString(ADD_ATTRIBUTE_FLAG, 0);
                    MString attrtype = argsdb.flagArgumentString(ADD_ATTRIBUTE_FLAG, 1);
                    long attrVtype = argsdb.flagArgumentInt(ADD_ATTRIBUTE_FLAG, 2);
                    if (attrname.isEmpty() || attrtype.isEmpty())
                    {
                        MGlobal::displayError("Attribute name and type must be specified.");
                        return MS::kFailure;
                    }

                    auto* attribute = cgraph->CreateAttributeFromTypeName(
                            attrname.asUTF8(), attrtype.asUTF8(), Gex::AttrValueType::Single,
                            Gex::AttrType(attrVtype));
                    if (!attribute)
                    {
                        MGlobal::displayError("Failed creating attribute "
                                              + attrname + " " + attrtype + " with type " + attrVtype);
                        return MS::kFailure;
                    }
                    attribute->SetInternal(true);
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
                            return MS::kFailure;
                        }

                        if (!dst)
                        {
                            MGlobal::displayError("Failed retrieving " + dstCnx + " attribute.");
                            return MS::kFailure;
                        }

                        if (!dst->ConnectSource(src))
                        {
                            MGlobal::displayWarning(("Failed connecting attributes " +
                                                     src->Longname() + " with name " + dst->Longname()).c_str());
                            return MS::kFailure;
                        }
                    }
                }
            }

            // Finalize callbacks on graph.
            if (removeCallback)
            {
                cgraph->DeregisterAttributeCallback(callback);
            }

            return MS::kSuccess;
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
//    else
//    {
//        MObject node = modifier.createNode(GexNetworkNode::id);
//        modifier.doIt();
//
//        MFnDependencyNode mfnnode(node);
//        if (names.length())
//        {
//            mfnnode.setName(names[0]);
//        }
//
//        setResult(mfnnode.name());
//    }


    return MStatus::kSuccess;
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

    GexMaya::LoadPlugin();

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

    Gex::PluginLoader::LoadPlugin("D:\\WORK\\GEX_MAYA\\GexMaya\\cmake-build-release\\MayaPlugin\\MayaPlugin\\Gex-plugins\\MathPlugin.dll");


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
