#include "MayaPlugin/include/Data.h"


#include "maya/MGlobal.h"
#include "maya/MArgList.h"


MTypeId GexMaya::GraphData::id = 0x7ffff + 8;


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

bool GexMaya::GraphData::IsGraphData(MPxData* other)
{
    return (other->typeId() == id);
}