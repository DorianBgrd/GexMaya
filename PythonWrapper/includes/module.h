#ifndef GEXMAYA_MODULE_H
#define GEXMAYA_MODULE_H

#include "Gex/include/Gex.h"
#include "boost/python.hpp"


namespace GexMaya
{
    boost::python::object ToGexNode(boost::python::tuple args,
                                    boost::python::dict kwargs);
}

#endif //GEXMAYA_MODULE_H
