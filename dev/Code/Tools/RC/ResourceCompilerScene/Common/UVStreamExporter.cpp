/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <Cry_Geo.h> // Needed for IIndexedMesh.h
#include <IIndexedMesh.h>
#include <AzCore/Math/MathUtils.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzToolsFramework/Debug/TraceContext.h>
#include <SceneAPI/SceneCore/Containers/Scene.h>
#include <SceneAPI/SceneCore/Containers/SceneGraph.h>
#include <SceneAPI/SceneCore/DataTypes/DataTypeUtilities.h>
#include <SceneAPI/SceneCore/DataTypes/Groups/IGroup.h>
#include <SceneAPI/SceneCore/DataTypes/Rules/IMeshAdvancedRule.h>
#include <SceneAPI/SceneCore/DataTypes/GraphData/IMeshVertexUVData.h>
#include <SceneAPI/SceneCore/DataTypes/Groups/ISkinGroup.h>
#include <SceneAPI/SceneCore/Utilities/Reporting.h>
#include <RC/ResourceCompilerScene/Common/CommonExportContexts.h>
#include <RC/ResourceCompilerScene/Common/UVStreamExporter.h>

namespace AZ
{
    namespace RC
    {
        namespace SceneEvents = AZ::SceneAPI::Events;
        namespace SceneDataTypes = AZ::SceneAPI::DataTypes;
        namespace SceneContainers = AZ::SceneAPI::Containers;
        namespace SceneUtilities = AZ::SceneAPI::Utilities;

        UVStreamExporter::UVStreamExporter()
        {
            BindToCall(&UVStreamExporter::CopyUVStream);
        }

        void UVStreamExporter::Reflect(ReflectContext* context)
        {
            SerializeContext* serializeContext = azrtti_cast<SerializeContext*>(context);
            if (serializeContext)
            {
                serializeContext->Class<UVStreamExporter, SceneAPI::SceneCore::ExportingComponent>()->Version(1);
            }
        }

        SceneEvents::ProcessingResult UVStreamExporter::CopyUVStream(MeshNodeExportContext& context) const
        {
            if (context.m_phase != Phase::Filling)
            {
                return SceneEvents::ProcessingResult::Ignored;
            }

            const SceneContainers::SceneGraph& graph = context.m_scene.GetGraph();
            const SceneDataTypes::IGroup& group = context.m_group;

            AZStd::shared_ptr<const SceneDataTypes::IMeshVertexUVData> uvs = nullptr;
            AZStd::string streamName;
            SceneEvents::ProcessingResultCombiner result;

            for (size_t uvIndex = 0; uvIndex < s_uvMaxStreamCount; ++uvIndex)
            {
                //try to populate a stream with a default stream matching its index order.
                SceneContainers::SceneGraph::NodeIndex index = graph.GetNodeChild(context.m_nodeIndex);
                size_t count = 0;
                while (index.IsValid())
                {
                    uvs = azrtti_cast<const SceneDataTypes::IMeshVertexUVData*>(graph.GetNodeContent(index));
                    if (uvs && count == uvIndex)
                    {
                        streamName = graph.GetNodeName(index).GetName();
                        break;
                    }
                    else if (uvs)
                    {
                        ++count;
                    }

                    index = graph.GetNodeSibling(index);
                }

                if (streamName.empty())
                {
                    AZ_TraceContext("UV set", "UVs not used");
                }
                else
                {
                    AZ_TraceContext("UV set", streamName);
                }

                if (uvs && context.m_mesh.GetVertexCount() != uvs->GetCount())
                {
                    AZ_TracePrintf(SceneUtilities::ErrorWindow,
                        "Number of vertices in the mesh (%i) doesn't match with the number of stored UVs (%i).",
                        context.m_mesh.GetVertexCount(), uvs->GetCount(), streamName.c_str());
                    result += SceneEvents::ProcessingResult::Failure;
                }

                // Populate the uv stream if it exists, or populate a default stream of 0's if the stream does not exist but the uvIndex is 0
                if (uvs != nullptr || uvIndex == 0)
                {
                    result += PopulateUVStream(context, uvIndex, uvs);
                }
            }
            return result.GetResult();
        }

        SceneEvents::ProcessingResult UVStreamExporter::PopulateUVStream(MeshNodeExportContext& context, int index, AZStd::shared_ptr<const SceneDataTypes::IMeshVertexUVData> uvs) const
        {
            context.m_mesh.ReallocStream(CMesh::TEXCOORDS, index, context.m_mesh.GetVertexCount());
            SMeshTexCoord* uvStream = context.m_mesh.template GetStreamPtr<SMeshTexCoord>(CMesh::TEXCOORDS, index);
            if (uvs)
            {
                for (int i = 0; i < context.m_mesh.GetVertexCount(); ++i)
                {
                    const AZ::Vector2& uv = uvs->GetUV(i);

                    if (!uv.IsFinite())
                    {
                        AZ_TracePrintf(SceneUtilities::ErrorWindow, "Invalid UV data detected at index %d.", i);
                        return SceneEvents::ProcessingResult::Failure;
                    }

                    // Note: If this is a skin mesh the y value of texture coordinate needs to be inverted, because as it processes 
                    // through CharacterCompiler::ProcessWork, it will get inverted again. This pre-corrects things to ensure the
                    // finally generated skin's uv texture coordinates are correct.
                    if (context.m_group.RTTI_IsTypeOf(AZ::SceneAPI::DataTypes::ISkinGroup::TYPEINFO_Uuid()))
                    {
                        uvStream[i] = SMeshTexCoord(uv.GetX(), 1.0f - uv.GetY());
                    }
                    else
                    {
                        uvStream[i] = SMeshTexCoord(uv.GetX(), uv.GetY());
                    }
                }
            }
            //Default to a dummy stream of data.
            else
            {
                static const SMeshTexCoord defaultTextureCoordinate(0.0f, 0.0f);
                for (int i = 0; i < context.m_mesh.GetVertexCount(); ++i)
                {
                    uvStream[i] = defaultTextureCoordinate;
                }
            }
            return SceneEvents::ProcessingResult::Success;
        }
    } // namespace RC
} // namespace AZ