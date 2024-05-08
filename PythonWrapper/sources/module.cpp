#include "PythonWrapper/includes/module.h"
#include "boost/python.hpp"

#include "maya/MPlug.h"
#include "maya/MGlobal.h"
#include "maya/MObject.h"
#include "maya/MPxData.h"
#include "maya/MFnPluginData.h"
#include "maya/MSelectionList.h"
#include "maya/MFnDependencyNode.h"

#include "MayaPlugin/include/defs.h"
#include "MayaPlugin/include/Data.h"




boost::python::object GexMaya::ToGexNode(
        boost::python::tuple args,
        boost::python::dict kwargs)
{
    std::string name = boost::python::extract<std::string>(args[0]);

    MSelectionList sel;
    sel.add(name.c_str());

    MObject node;
    sel.getDependNode(0, node);

    if (node.isNull())
    {
        MGlobal::displayError("Provided node is invalid.");
        return {};
    }

    MFnDependencyNode depnode(node);
    MPlug plug = depnode.findPlug(GEX_GRAPH_ATTR, true);
    if (plug.isNull())
    {
        MGlobal::displayError("Graph plug was not found.");
        return {};
    }

    MFnPluginData pluginData(plug.asMObject());

    MPxData* data =  pluginData.data();
    if (!data)
    {
        MGlobal::displayWarning("Plug graph data is empty.");
        return {};
    }

    auto* gdata = dynamic_cast<GexMaya::GraphData*>(data);
    if (!gdata)
    {
        MGlobal::displayError("Plug graph data is invalid.");
        return {};
    }

    return boost::python::object(
            boost::python::ptr(gdata->Graph())
            );
}


BOOST_PYTHON_MODULE(GexMayaPython)
{
    boost::python::def(
            "ToGexNode", boost::python::raw_function(
                    GexMaya::ToGexNode, 1));
}