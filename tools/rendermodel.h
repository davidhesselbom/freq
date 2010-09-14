#ifndef RENDERMODEL_H
#define RENDERMODEL_H

#include "sawe/project.h"
#include "heightmap/collection.h"

namespace Tools
{
    class RenderModel
    {
    public:
        RenderModel(Sawe::Project* p);

        Heightmap::pCollection collection;
        Signal::pWorkerCallback collectionCallback;

    private:
        friend class RenderView; // todo remove
        Sawe::Project* project;
    };
} // namespace Tools

#endif // RENDERMODEL_H
