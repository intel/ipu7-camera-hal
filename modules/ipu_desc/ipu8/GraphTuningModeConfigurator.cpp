#include "GraphResolutionConfiguratorInclude.h"
#include "GraphTuningModeConfigurator.h"

#if SUPPORT_TM_SWITCH == 1
bool GraphTuningModeConfigurator::SwitchTuningModeBasedOnKey(uint8_t tuningModeKey)
{
    GraphTopology* graphTopology = nullptr;
    _staticGraph->getGraphTopology(&graphTopology);
    SubGraphInnerNodeConfiguration innerNodeConfig = {};
    bool modeSwitched = false;
    for (int i = 0; i < graphTopology->numOfLinks; i++)
    {
        auto& link = graphTopology->links[i];
        for (int j = 0; j < 4; ++j)
        {
            if (link->srcNode != nullptr && link->srcNode->nodeTuningModes[j].key == tuningModeKey && link->srcNode->nodeKernels.operationMode != link->srcNode->nodeTuningModes[j].id)
            {
                link->srcNode->nodeKernels.operationMode = link->srcNode->nodeTuningModes[j].id;
                modeSwitched = true;
            }
            if (link->destNode != nullptr && link->destNode->nodeTuningModes[j].key == tuningModeKey && link->destNode->nodeKernels.operationMode != link->destNode->nodeTuningModes[j].id)
            {
                link->destNode->nodeKernels.operationMode = link->destNode->nodeTuningModes[j].id;
                modeSwitched = true;
            }
        }
    }
    return modeSwitched;
}
#endif