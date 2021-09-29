//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashColladaLoader.h"
#include "MashDevice.h"
#include "MashVideo.h"
#include "MashMaterial.h"
#include "MashMaterialManager.h"
#include "MashControllerManager.h"
#include "MashSceneNode.h"
#include "MashSceneManager.h"
#include "MashMemory.h"
#include "MashMesh.h"
#include "MashStaticMesh.h"
#include "MashEntity.h"
#include "MashDummy.h"
#include "MashCamera.h"
#include "MashLight.h"
#include "MashBone.h"
#include "MashSkin.h"
#include "MashAnimationBuffer.h"
#include "MashKeySet.h"
#include "MashMeshBuilder.h"
#include "MashHelper.h"
#include "MashModel.h"
#include "MashFileManager.h"
#include "MashGenericArray.h"
#include "MashStringHelper.h"
#include "MashLog.h"
#include <set>
#include <cctype>
namespace mash
{
	static const uint32 g_MemPoolTypeSize = 160000000;

	CMashColladaLoader::CMashColladaLoader():m_memoryPool(g_MemPoolTypeSize)
	{
		
	}

	void CMashColladaLoader::ConvertTextArrayToElements(const int8 *str, sVariableArray &out, uint32 writeOffset, uint32 *elementsWritten)
	{
		if (elementsWritten)
			*elementsWritten = 0;

		uint32 currentIndex = 0;
		const uint32 maxBufferCount = 20;
		int8 currentNumberBuffer[maxBufferCount];//data shouldn't exceed this
		uint32 currentNumberBufferIndex = 0;
		uint32 arrayIndex = 0;

		if (!str)
			return;

		if ((out.count == 0) || (str[0] == 0))
			return;

		while(str[currentIndex] != 0)
		{
			if (isspace(str[currentIndex]) && (currentNumberBufferIndex > 0))
			{
				//null terminator
				currentNumberBuffer[currentNumberBufferIndex] = 0;

				MASH_ASSERT((writeOffset + arrayIndex) < out.count);

				switch(out.type)
				{
				case aARRAY_DATA_SOURCE_FLOAT:
					out.f[writeOffset + arrayIndex] = atof(currentNumberBuffer);
					++arrayIndex;
					break;
				case aARRAY_DATA_SOURCE_INT:
					out.i[writeOffset + arrayIndex] = atoi(currentNumberBuffer);
					++arrayIndex;
					break;
				case aARRAY_DATA_SOURCE_BOOL:
					{
						if (currentNumberBuffer[0] == 't')
							out.b[writeOffset + arrayIndex] = true;
						else
							out.b[writeOffset + arrayIndex] = false;

						++arrayIndex;
					}
					break;
				}

				currentNumberBufferIndex = 0;
			}
			else
			{
				MASH_ASSERT(currentNumberBufferIndex < maxBufferCount);
				currentNumberBuffer[currentNumberBufferIndex++] = str[currentIndex];
			}

			++currentIndex;
		}

		//grab the last number
		if (currentNumberBufferIndex > 0)
		{
			//null terminator
			currentNumberBuffer[currentNumberBufferIndex] = 0;

			MASH_ASSERT((writeOffset + arrayIndex) < out.count);

			switch(out.type)
			{
			case aARRAY_DATA_SOURCE_FLOAT:
				out.f[writeOffset + arrayIndex] = atof(currentNumberBuffer);
				++arrayIndex;
				break;
			case aARRAY_DATA_SOURCE_INT:
				out.i[writeOffset + arrayIndex] = atoi(currentNumberBuffer);
				++arrayIndex;
				break;
			case aARRAY_DATA_SOURCE_BOOL:
				{
					if (currentNumberBuffer[0] == 't')
						out.b[writeOffset + arrayIndex] = true;
					else
						out.b[writeOffset + arrayIndex] = false;

					++arrayIndex;
				}
				break;
			}
		}

		if (elementsWritten)
			*elementsWritten = arrayIndex;
	}

	void CMashColladaLoader::GetInputSemanticData(MashXMLReader *xmlNode, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap, MashList<sInputData> &out, MashList<sInputData> *vertexInputs)
	{
		sInputData newElement;
		newElement.usage = aEXT_INPUT_SEMANTIC_COUNT;
		newElement.indexOffset = 0;

		const int8 *semanticString = xmlNode->GetAttributeRaw("semantic");
		if (strcmp(semanticString, "POSITION") == 0)
			newElement.usage = aDECLUSAGE_POSITION;
		else if (strcmp(semanticString, "NORMAL") == 0)
			newElement.usage = aDECLUSAGE_NORMAL;
		else if (strcmp(semanticString, "TEXCOORD") == 0)
			newElement.usage = aDECLUSAGE_TEXCOORD;
		else if (strcmp(semanticString, "TEXTANGENT") == 0)
			newElement.usage = aDECLUSAGE_TANGENT;
		else if (strcmp(semanticString, "WEIGHT") == 0)
			newElement.usage = aEXT_INPUT_SEMANTIC_WEIGHT;
		else if (strcmp(semanticString, "JOINT") == 0)
			newElement.usage = aEXT_INPUT_SEMANTIC_JOINT;
		else if (strcmp(semanticString, "INV_BIND_MATRIX") == 0)
			newElement.usage = aEXT_INPUT_SEMANTIC_INV_BIND_MATRIX;
		else if (strcmp(semanticString, "TEXBINORMAL") == 0)
		{
			//not used
		}
		else if (strcmp(semanticString, "VERTEX") == 0)
		{
			//special case. This should evaluate to POSITION
			if (vertexInputs)
			{
				MashList<sInputData>::Iterator iter = vertexInputs->Begin();
				MashList<sInputData>::Iterator end = vertexInputs->End();
				for(; iter != end; ++iter)
				{
					if (iter->usage == aDECLUSAGE_POSITION)
					{
						xmlNode->GetAttributeInt("offset", iter->indexOffset);
						out.PushBack(*iter);
						return;
					}
				}
			}

			//shouldn't happen
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
				"Failed to find POSITION element in VERTEX input.",
				"CMashColladaLoader::GetInputSemanticData");
		}

		//non supported element
		if (newElement.usage == aEXT_INPUT_SEMANTIC_COUNT)
			return;

		//if there is no source then we skip this element
		MashStringc sourceArray;
		if (!xmlNode->GetAttributeString("source", sourceArray))
			return;

		if (sourceArray[0] == '#')
			sourceArray.Erase(sourceArray.Begin());

		std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc >::iterator sourceIter = sourceMap.find(sourceArray);
		if (sourceIter == sourceMap.end())
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
						"CMashColladaLoader::GetInputSemanticData",
						"Failed to find source array.",
						sourceArray.GetCString());
			return;
		}

		newElement.sourceArray = &sourceIter->second;

		xmlNode->GetAttributeInt("offset", newElement.indexOffset);

		out.PushBack(newElement);
	}

	void CMashColladaLoader::DecodeName(const int8 *str, MashStringc &nameOut, int32 &bitFlagOut, int32 &flagIdOut)
	{
		flagIdOut = -1;
		bitFlagOut = 0;

		MashStringc codeString;
		nameOut.Clear();
		bool codeFlagFound = false;
		bool interpreted = false;
		uint32 i = 0;
		while(str[i] != 0)
		{
			if (codeFlagFound)
			{
				if (interpreted && isdigit(str[i]))
				{
					flagIdOut = str[i] - '0';
					break;//job done
				}
				else if (isalpha(str[i]))
				{
					codeString += tolower(str[i]);

					if (codeString == "lod")
					{
						bitFlagOut |= aNAME_FLAG_LOD;
						interpreted = true;
						codeString.Clear();
					}

					//TODO : Add any new flags here
				}
			}
			else
			{
				//if (i+1 < strLength)
				if (str[i+1] != 0)
				{
					if ((str[i] == '_') && (str[i+1] == '_'))
						codeFlagFound = true;
				}
				
				if (!codeFlagFound)
				{
					nameOut += str[i];
				}
			}

			++i;
		}
	}

	void CMashColladaLoader::ConvertTexCoord2(f32 *out)
	{
		out[1] = 1.0f - out[1];
	}

	void CMashColladaLoader::ConvertVector3(eFILE_UP_AXIS upAxis, f32 *out)
	{
		//return;
		if (upAxis == aFILE_UP_AXIS_X)
		{
			/*
				right axis = -y
				up axis = +x
				in axis = +z
			*/
			f32 t = out[0];
			out[0] = out[1];
			out[1] = t;
		}
		else if (upAxis == aFILE_UP_AXIS_Z)
		{
			/*
				right axis = +x
				up axis = +z
				in axis = -y
			*/
			f32 t = out[1];
			out[1] = out[2];
			out[2] = t;
		}
		else
		{
			/*
				right axis = +x
				up axis = +y
				in axis = +z
			*/
		}
	}

	void CMashColladaLoader::ConvertMatrix(eFILE_UP_AXIS upAxis, mash::MashMatrix4 &mat)
	{
		//return;
		if (upAxis == aFILE_UP_AXIS_X)
		{
			/*
				right axis = -y
				up axis = +x
				in axis = +z
			*/
		}
		else if (upAxis == aFILE_UP_AXIS_Z)
		{
			/*
				right axis = +x
				up axis = +z
				in axis = -y
			*/
			mash::MashMatrix4 temp = mat;
			temp.Transpose();
			temp.v[1]=mat.v[8];
			temp.v[2]=mat.v[4];
			temp.v[4]=mat.v[2];
			temp.v[5]=mat.v[10];
			temp.v[6]=mat.v[6];
			temp.v[8]=mat.v[1];
			temp.v[9]=mat.v[9];
			temp.v[10]=mat.v[5];
			temp.v[12]=mat.v[3];
			temp.v[13]=mat.v[11];
			temp.v[14]=mat.v[7];

			mat = temp;
		}
		else
		{
			mat.Transpose();
		}
	}

	void CMashColladaLoader::ConvertTexCoordArrayAxisIfNeeded(sArrayDataSource *arrayData)
	{
		if (!arrayData->isAxisConverted && (arrayData->varaibleArray.type == aARRAY_DATA_SOURCE_FLOAT))
		{
			if (arrayData->stride > 1)
			{
				for(uint32 vec = 0; vec < arrayData->count; ++vec)
				{
					//convert in place
					ConvertTexCoord2(&arrayData->varaibleArray.f[vec * arrayData->stride]);
				}
			}
		}

		arrayData->isAxisConverted = true;
	}

	void CMashColladaLoader::ConvertVectorArrayAxisIfNeeded(eFILE_UP_AXIS upAxis, sArrayDataSource *arrayData)
	{
		if (!arrayData->isAxisConverted && (upAxis != aFILE_UP_AXIS_Y) && (arrayData->varaibleArray.type == aARRAY_DATA_SOURCE_FLOAT))
		{
			if (arrayData->stride > 2)
			{
				for(uint32 vec = 0; vec < arrayData->count; ++vec)
				{
					//convert in place
					ConvertVector3(upAxis, &arrayData->varaibleArray.f[vec * arrayData->stride]);
				}
			}
		}

		arrayData->isAxisConverted = true;
	}

	CMashColladaLoader::eANIM_CHANNEL_TARGET CMashColladaLoader::ConvertFloatArrayIfNeeded(eFILE_UP_AXIS upAxis, eANIM_CHANNEL_TARGET currentTarget, sArrayDataSource *arrayData)
	{
		eANIM_CHANNEL_TARGET newTarget = currentTarget;
		if (arrayData && !arrayData->isAxisConverted && (upAxis != aFILE_UP_AXIS_Y) && (arrayData->varaibleArray.type == aARRAY_DATA_SOURCE_FLOAT))
		{
			if (upAxis == aFILE_UP_AXIS_Z)
			{
				switch(currentTarget)
				{
				case aANIM_CHANNEL_TRANSLATION_Y:
					newTarget = aANIM_CHANNEL_TRANSLATION_Z;
					break;
				case aANIM_CHANNEL_TRANSLATION_Z:
					newTarget = aANIM_CHANNEL_TRANSLATION_Y;
					break;

				case aANIM_CHANNEL_ROTATION_Y:
					newTarget = aANIM_CHANNEL_ROTATION_Z;
					break;
				case aANIM_CHANNEL_ROTATION_Z:
					newTarget = aANIM_CHANNEL_ROTATION_Y;
					break;

				case aANIM_CHANNEL_SCALE_Y:
					newTarget = aANIM_CHANNEL_SCALE_Z;
					break;
				case aANIM_CHANNEL_SCALE_Z:
					newTarget = aANIM_CHANNEL_SCALE_Y;
					break;

				case aANIM_CHANNEL_MATRIX:
					{
						uint32 matrixCount = arrayData->count;
						const uint32 elmsToCopy = math::Clamp<uint32>(0, 16, arrayData->stride);
						for(uint32 m = 0; m < matrixCount; ++m)
						{
							mash::MashMatrix4 mtemp;
							memcpy(mtemp.v, &arrayData->varaibleArray.f[m * arrayData->stride], sizeof(f32) * elmsToCopy);
							ConvertMatrix(upAxis, mtemp);
							memcpy(&arrayData->varaibleArray.f[m * arrayData->stride], mtemp.v, sizeof(f32) * elmsToCopy);
						}
						break;
					}
				}
			}
		}

		arrayData->isAxisConverted = true;
		return newTarget;
	}

	void CMashColladaLoader::GetMatrixFromArray(const int8 *str, eFILE_UP_AXIS upAxis, mash::MashMatrix4 &out)
	{
		GetFloatsFromArray(str, out.v, 16);
		ConvertMatrix(upAxis, out);
	}

	void CMashColladaLoader::GetVec3FromArray(const int8 *str, eFILE_UP_AXIS upAxis, f32 *out)
	{
		GetFloatsFromArray(str, out, 3);
		ConvertVector3(upAxis, out);
	}

	void CMashColladaLoader::GetTexCoord2FromArray(const int8 *str, f32 *out)
	{
		GetFloatsFromArray(str, out, 2);
		ConvertTexCoord2(out);
	}

	void CMashColladaLoader::GetRotationAxisAngleFromArray(const int8 *str, eFILE_UP_AXIS upAxis, mash::MashMatrix4 &out)
	{
		f32 f[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		GetFloatsFromArray(str, f, 4);

		mash::MashQuaternion rot;
		rot.SetRotationAxis(mash::MashVector3(f[0], f[2], f[1]), math::DegsToRads(f[3]));
		rot.ToMatrix(out);
		out.Transpose();
	}

	void CMashColladaLoader::GetFloatsFromArray(const int8 *str, f32 *floatAry, uint32 floatCount)
	{
		uint32 currentIndex = 0;
		int8 currentNumberBuffer[20];//data shouldn't exceed this
		uint32 currentNumberBufferIndex = 0;
		uint32 arrayIndex = 0;

		if (!str || (str[0] == 0))
			return;

		while(str[currentIndex] != 0)
		{
			if (isspace(str[currentIndex]) && (currentNumberBufferIndex > 0))
			{
				//null terminator
				currentNumberBuffer[currentNumberBufferIndex] = 0;

				MASH_ASSERT(arrayIndex < floatCount);
				floatAry[arrayIndex++] = atof(currentNumberBuffer);

				currentNumberBufferIndex = 0;
			}
			else
			{
				currentNumberBuffer[currentNumberBufferIndex++] = str[currentIndex];
			}

			++currentIndex;
		}

		//grab the last number
		if (currentNumberBufferIndex > 0)
		{
			//null terminator
			currentNumberBuffer[currentNumberBufferIndex] = 0;
			MASH_ASSERT(arrayIndex < floatCount);
			floatAry[arrayIndex++] = atof(currentNumberBuffer);
		}
	}

	const int8* CMashColladaLoader::RemoveStringHash(const int8 *s)
	{
		if (s && (s[0] == '#'))
			++s;

		return s;
	}

	bool CMashColladaLoader::ReadNodeElements(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, sNode *newNode, bool geometryOffsetNodeEntered, MashList<sNode> &nodesOut)
	{
		if (xmlReader->MoveToFirstChild())
		{
			do
			{
				if (strcmp(xmlReader->GetNameRaw(), "matrix") == 0)
				{
					if (!geometryOffsetNodeEntered)
						GetMatrixFromArray(xmlReader->GetTextRaw(), upAxis, newNode->localTransform);
					else
						GetMatrixFromArray(xmlReader->GetTextRaw(), upAxis, newNode->geometryOffsetTransform);
				}
				else if (strcmp(xmlReader->GetNameRaw(), "rotate") == 0)
				{
					mash::MashMatrix4 m;
					GetRotationAxisAngleFromArray(xmlReader->GetTextRaw(), upAxis, m);
					
					if (!geometryOffsetNodeEntered)
						newNode->localTransform = m * newNode->localTransform;
					else
						newNode->geometryOffsetTransform = m * newNode->geometryOffsetTransform;
				}
				else if (strcmp(xmlReader->GetNameRaw(), "translate") == 0)
				{
					mash::MashVector3 translate;
					GetVec3FromArray(xmlReader->GetTextRaw(), upAxis, translate.v);

					if (!geometryOffsetNodeEntered)
						newNode->localTransform.SetTranslation(translate);
					else
						newNode->geometryOffsetTransform.SetTranslation(translate);
				}
				else if (strcmp(xmlReader->GetNameRaw(), "instance_controller") == 0)
				{
					newNode->controllerName = xmlReader->GetAttributeRaw("url");
					newNode->controllerName = RemoveStringHash(newNode->controllerName);

					if (xmlReader->MoveToFirstChild("skeleton"))
					{
						newNode->skeletonRootNodeName = xmlReader->GetAttributeRaw("url");
						newNode->skeletonRootNodeName = RemoveStringHash(newNode->skeletonRootNodeName);

						xmlReader->PopChild();
					}
				}
				else if (strcmp(xmlReader->GetNameRaw(), "instance_geometry") == 0)
				{
					newNode->geometryName = xmlReader->GetAttributeRaw("url");
					newNode->geometryName = RemoveStringHash(newNode->geometryName);
				}
				else if (strcmp(xmlReader->GetNameRaw(), "instance_light") == 0)
				{
					newNode->lightName = xmlReader->GetAttributeRaw("url");
					newNode->lightName = RemoveStringHash(newNode->lightName);
				}
				else if (strcmp(xmlReader->GetNameRaw(), "node") == 0)
				{
					if (!xmlReader->GetAttributeRaw("name") || 
						(strstr(xmlReader->GetAttributeRaw("name"), newNode->nameFromFile) && 
						strstr(xmlReader->GetAttributeRaw("name"), "-Pivot")))
					{
						/*
							In openCollada, a sub node with no name means a geometry offset.
							In 3dsmax, an appended '-Pivot' mean a geometry offset
						*/
						if (ReadNodeElements(xmlReader, upAxis, newNode, true, nodesOut))
                        {
							//hasAnyTransform = true;
                        }
					}
					else
					{
						//recurse into child elements
						GetNodeData(xmlReader, upAxis, newNode, nodesOut);
					}
				}

			}while(xmlReader->MoveToNextSibling());

			xmlReader->PopChild();
		}

		return true;
	}

	void CMashColladaLoader::GetNodeData(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, sNode *parentNode, MashList<sNode> &nodesOut)
	{
		sNode *prevSibling = 0;
		MashStringc decodedName;
		do
		{
			const int8 *nodeId = xmlReader->GetAttributeRaw("id");
			const int8 *nodeName = xmlReader->GetAttributeRaw("name");

			decodedName.Clear();
			bool processNode = true;

			/*
				If a node has a LOD id greater than 0 then we dont process it. It is assumed
				if LOD codes are used, there exists a node with LOD 0.
			*/
			if (nodeName)
			{
				int32 nameFlags, flagNumbers;
				DecodeName(nodeName, decodedName, nameFlags, flagNumbers);

				if ((nameFlags & aNAME_FLAG_LOD) && (flagNumbers > 0))
					processNode = false;
			}

			if (processNode && (nodeId || nodeName))
			{
				MashList<sNode>::Iterator newNodeIter = nodesOut.Insert(nodesOut.End(), sNode());

				if (prevSibling)
					prevSibling->nextSibling = prevSibling;

				sNode *newNode = &(*newNodeIter);
				newNode->id = nodeId;
				newNode->nameFromFile = nodeName;
				newNode->parentNode = parentNode;
				newNode->decodedName = decodedName;
				newNode->nextSibling = 0;
				newNode->jointName = 0;
				newNode->controllerName = 0;
				newNode->skeletonRootNodeName = 0;
				newNode->geometryName = 0;
				newNode->lightName = 0;
				newNode->cameraName = 0;
				newNode->engineNode = 0;

				newNode->localTransform.Identity();
				newNode->geometryOffsetTransform.Identity();

				newNode->nodeType = aCOLLADA_NODE_TYPE_NODE;
				const int8 *nodeType = xmlReader->GetAttributeRaw("type");
				if (nodeType)
				{
					if (strcmp(nodeType, "JOINT") == 0)
					{
						newNode->nodeType = aCOLLADA_NODE_TYPE_JOINT;
						newNode->jointName = xmlReader->GetAttributeRaw("sid");
						//if no sid then just use the given name. Some exporters work this way.
						if (!newNode->jointName)
							newNode->jointName = nodeName;
					}
				}

				

				if (!ReadNodeElements(xmlReader, upAxis, newNode, false, nodesOut))
				{
					nodesOut.Erase(newNodeIter);
				}

			}
			else
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
					"Node with no identifier found. Loading will be skipped.",
					"CMashColladaLoader::Load");
			}

		}while(xmlReader->MoveToNextSibling("node"));
	}

	eMASH_STATUS CMashColladaLoader::GetSourceElements(MashXMLReader *xmlReader, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap)
	{
		f32 errorRaised = false;

		sSource newArraySource;
		newArraySource.id = xmlReader->GetAttributeRaw("id");
		newArraySource.arrayData.isAxisConverted = false;

		if (xmlReader->MoveToFirstChild())
		{
			if (strcmp(xmlReader->GetNameRaw(), "asset") == 0)
			{
				//TODO
				xmlReader->MoveToNextSibling();
			}
			
			if (strcmp(xmlReader->GetNameRaw(), "float_array") == 0)
			{
				if (xmlReader->GetAttributeInt("count", newArraySource.arrayData.varaibleArray.count))
				{
					newArraySource.arrayData.id = xmlReader->GetAttributeRaw("id");
					newArraySource.arrayData.varaibleArray.type = aARRAY_DATA_SOURCE_FLOAT;
					//newArraySource.arrayData.varaibleArray.f = MASH_ALLOC_T_COMMON(f32, newArraySource.arrayData.varaibleArray.count);
					newArraySource.arrayData.varaibleArray.f = (f32*)m_memoryPool.GetMemory(sizeof(f32) * newArraySource.arrayData.varaibleArray.count);

					ConvertTextArrayToElements(xmlReader->GetTextRaw(), newArraySource.arrayData.varaibleArray);
				}
				else
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
						"Array does not contain a 'count' element. This array is assumed empty.",
						"CMashColladaLoader::GetSourceElements");
				}

				xmlReader->MoveToNextSibling();
			}
			else if (strcmp(xmlReader->GetNameRaw(), "int_array") == 0)
			{
				if (xmlReader->GetAttributeInt("count", newArraySource.arrayData.varaibleArray.count))
				{
					newArraySource.arrayData.id = xmlReader->GetAttributeRaw("id");
					newArraySource.arrayData.varaibleArray.type = aARRAY_DATA_SOURCE_INT;
					newArraySource.arrayData.varaibleArray.i = (int32*)m_memoryPool.GetMemory(sizeof(int32) * newArraySource.arrayData.varaibleArray.count);

					ConvertTextArrayToElements(xmlReader->GetTextRaw(), newArraySource.arrayData.varaibleArray);
				}
				xmlReader->MoveToNextSibling();
			}
			else if (strcmp(xmlReader->GetNameRaw(), "bool_array") == 0)
			{
				if (xmlReader->GetAttributeInt("count", newArraySource.arrayData.varaibleArray.count))
				{
					newArraySource.arrayData.id = xmlReader->GetAttributeRaw("id");
					newArraySource.arrayData.varaibleArray.type = aARRAY_DATA_SOURCE_BOOL;
					newArraySource.arrayData.varaibleArray.b = (bool*)m_memoryPool.GetMemory(sizeof(bool) * newArraySource.arrayData.varaibleArray.count);

					ConvertTextArrayToElements(xmlReader->GetTextRaw(), newArraySource.arrayData.varaibleArray);
				}
				else
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
						"Array does not contain a 'count' element. This array is assumed empty.",
						"CMashColladaLoader::GetSourceElements");
				}

				xmlReader->MoveToNextSibling();
			}
			else if (strcmp(xmlReader->GetNameRaw(), "Name_array") == 0)
			{
				if (xmlReader->GetAttributeInt("count", newArraySource.arrayData.varaibleArray.count))
				{
					newArraySource.arrayData.id = xmlReader->GetAttributeRaw("id");
					newArraySource.arrayData.varaibleArray.type = aARRAY_DATA_SOURCE_NAME;
					newArraySource.arrayData.varaibleArray.s = xmlReader->GetTextRaw();
				}
				else
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
						"Array does not contain a 'count' element. This array is assumed empty.",
						"CMashColladaLoader::GetSourceElements");
				}

				xmlReader->MoveToNextSibling();
			}

			if (strcmp(xmlReader->GetNameRaw(), "technique_common") == 0)
			{
				if (xmlReader->MoveToFirstChild("accessor"))
				{
					/*
						NOTE : The following assumes the accessor is accessing the data
						array just loaded. This could be wrong as arrays arn't necessarily scoped. 
						Instead think about using a map and searching it.
					*/
					//xmlReader->GetAttributeRaw("source");

					newArraySource.arrayData.stride = 0;
					newArraySource.arrayData.offset = 0;
					newArraySource.arrayData.count = 0;
					
					if (!xmlReader->GetAttributeInt("stride", newArraySource.arrayData.stride))
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
							"Collada accessor does not contain a 'stride' element. This accessor is assumed empty.",
							"CMashColladaLoader::GetSourceElements");
					}

					xmlReader->GetAttributeInt("offset", newArraySource.arrayData.offset);

					if (!xmlReader->GetAttributeInt("count", newArraySource.arrayData.count))
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
							"Collada accessor does not contain a 'count' element. This accessor is assumed empty.",
							"CMashColladaLoader::GetSourceElements");
					}

					if (newArraySource.arrayData.stride > g_maxColladaAccessorParams)
					{
						MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
							"CMashColladaLoader::Load",
							"Collada accessor params larger than fixed allocated size. %d.",
							newArraySource.arrayData.stride);

						newArraySource.arrayData.stride = 0;
						errorRaised = true;
					}

					//set all elements to zero
					memset(newArraySource.arrayData.paramValid, 0, sizeof(newArraySource.arrayData.paramValid));

					/*
						As per the Collada documentation under "accessor", the names of params are not
						significant and do not imply anything. They only have meaning if there is no
						name, meaning that element is to be skipped.
					*/
					if ((newArraySource.arrayData.stride > 0) && xmlReader->MoveToFirstChild("param"))
					{
						uint32 currentStride = 0;
						do
						{
							/*
								Simple bit value to state if we should skip an element.
								If there is no name then it will be skipped when creating
								the vertex array
							*/
							if (xmlReader->GetAttributeRaw("name"))
								newArraySource.arrayData.paramValid[currentStride++] = 1;
							else
								newArraySource.arrayData.paramValid[currentStride++] = 0;

						}while(xmlReader->MoveToNextSibling());

						xmlReader->PopChild();
					}

					xmlReader->PopChild();
				}

				xmlReader->MoveToNextSibling();
			}

			if (newArraySource.arrayData.varaibleArray.count != 0)
			{
				sourceMap[newArraySource.id] = newArraySource;
			}

			xmlReader->PopChild();
		}

		if (errorRaised)
			return aMASH_FAILED;

		return aMASH_OK;
	}

	void CMashColladaLoader::GetAnimSamplerData(MashXMLReader *xmlNode, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap, std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc > &animSamplerMap)
	{
		std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc >::iterator newElementIter = animSamplerMap.insert(std::make_pair(xmlNode->GetAttributeRaw("id"), sAnimSampler())).first;
		memset(newElementIter->second.inputSource, 0, sizeof(newElementIter->second.inputSource));
        
		if (xmlNode->MoveToFirstChild("input"))
		{
			do
			{
				const int8 *semanticString = xmlNode->GetAttributeRaw("semantic");
				eANIM_INPUT_SEMANTICS inputSemanticType = aANIM_INPUT_SEMANTIC_COUNT;
				if (strcmp(semanticString, "INPUT") == 0)
				{
					inputSemanticType = aANIM_INPUT_SEMANTIC_INPUT;
				}
				else if (strcmp(semanticString, "OUTPUT") == 0)
				{
					inputSemanticType = aANIM_INPUT_SEMANTIC_OUTPUT;
				}
				else if (strcmp(semanticString, "INTERPOLATION") == 0)
				{
					inputSemanticType = aANIM_INPUT_SEMANTIC_INTERPOLATION;
				}

				if (inputSemanticType != aANIM_INPUT_SEMANTIC_COUNT)
				{
					const int8 *sourceString = RemoveStringHash(xmlNode->GetAttributeRaw("source"));
					if (sourceString)
					{
						std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc >::iterator sourceIter = sourceMap.find(sourceString);
						if (sourceIter != sourceMap.end())
							newElementIter->second.inputSource[inputSemanticType] = &sourceIter->second;
					}
				}

			}while(xmlNode->MoveToNextSibling("input"));

			xmlNode->PopChild();
		}
	}

	void CMashColladaLoader::GetAnimChannelData(MashXMLReader *xmlNode, eFILE_UP_AXIS upAxis, std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc > &animSamplerMap, std::map<MashStringc, sAnimChannel, std::less<MashStringc>, animChannelAlloc > &animChannelMap)
	{
		const int8 *sourceString = RemoveStringHash(xmlNode->GetAttributeRaw("source"));
		//decode target string
		const int8 *targetString = xmlNode->GetAttributeRaw("target");
		const int8 *seperatorToken = strchr(targetString, '/');
		if (seperatorToken)
		{
			MashStringc targetOwnerName = targetString;
			MashStringc targetOp;
			targetOwnerName.Erase(targetOwnerName.Begin() +  (seperatorToken - targetString), targetOwnerName.End());
			targetOp = &targetString[(seperatorToken - targetString) + 1];

			std::map<MashStringc, sAnimChannel, std::less<MashStringc>, animChannelAlloc >::iterator ownerChannelMapIter = animChannelMap.find(targetOwnerName);
			if (ownerChannelMapIter == animChannelMap.end())
			{
				ownerChannelMapIter = animChannelMap.insert(std::make_pair(targetOwnerName, sAnimChannel())).first;
				memset(ownerChannelMapIter->second.samplers, 0, sizeof(ownerChannelMapIter->second.samplers));
			}

			eANIM_CHANNEL_TARGET channelTarget = aANIM_CHANNEL_COUNT;
			//Decode targetOp
			if (strcmp(targetOp.GetCString(), "matrix") == 0)
				channelTarget = aANIM_CHANNEL_MATRIX;
			else if ((strcmp(targetOp.GetCString(), "translation.X") == 0) || (strcmp(targetOp.GetCString(), "translate.X") == 0))
				channelTarget = aANIM_CHANNEL_TRANSLATION_X;
			else if ((strcmp(targetOp.GetCString(), "translation.Y") == 0) || (strcmp(targetOp.GetCString(), "translate.Y") == 0))
				channelTarget = aANIM_CHANNEL_TRANSLATION_Y;
			else if ((strcmp(targetOp.GetCString(), "translation.Z") == 0) || (strcmp(targetOp.GetCString(), "translate.Z") == 0))
				channelTarget = aANIM_CHANNEL_TRANSLATION_Z;
			else if (strcmp(targetOp.GetCString(), "scale.X") == 0)
				channelTarget = aANIM_CHANNEL_SCALE_X;
			else if (strcmp(targetOp.GetCString(), "scale.Y") == 0)
				channelTarget = aANIM_CHANNEL_SCALE_Y;
			else if (strcmp(targetOp.GetCString(), "scale.Z") == 0)
				channelTarget = aANIM_CHANNEL_SCALE_Z;
			else if ((strcmp(targetOp.GetCString(), "rotationX.ANGLE") == 0) || (strcmp(targetOp.GetCString(), "rotateX.ANGLE") == 0))
				channelTarget = aANIM_CHANNEL_ROTATION_X;
			else if ((strcmp(targetOp.GetCString(), "rotationY.ANGLE") == 0) || (strcmp(targetOp.GetCString(), "rotateY.ANGLE") == 0))
				channelTarget = aANIM_CHANNEL_ROTATION_Y;
			else if ((strcmp(targetOp.GetCString(), "rotationZ.ANGLE") == 0) || (strcmp(targetOp.GetCString(), "rotateZ.ANGLE") == 0))
				channelTarget = aANIM_CHANNEL_ROTATION_Z;

			std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc >::iterator samplerIter = animSamplerMap.find(sourceString);
			if ((channelTarget != aANIM_CHANNEL_COUNT) && (samplerIter != animSamplerMap.end()))
			{
				channelTarget = ConvertFloatArrayIfNeeded(upAxis, channelTarget, &samplerIter->second.inputSource[aANIM_INPUT_SEMANTIC_OUTPUT]->arrayData);
				ownerChannelMapIter->second.samplers[channelTarget] = &samplerIter->second;
			}
		}
	}

	MashAnimationBuffer* CMashColladaLoader::CreateAnimationBuffer(MashDevice *device, sAnimChannel *animChannel, sNode *node, uint32 frameRate)
	{
		/*
			First we do a bit of validation...
		*/
		for(uint32 i = 0; i < aANIM_CHANNEL_COUNT; ++i)
		{
			if (animChannel->samplers[i])
			{
				sSource *sourceInterpolation = animChannel->samplers[i]->inputSource[aANIM_INPUT_SEMANTIC_INTERPOLATION];
				if (sourceInterpolation)
				{
					uint32 lengthOfFirstWord = strcspn(sourceInterpolation->arrayData.varaibleArray.s, " /t");
					MashStringc interpolationType(sourceInterpolation->arrayData.varaibleArray.s, lengthOfFirstWord);
					if (sourceInterpolation && (strcmp(interpolationType.GetCString(), "LINEAR") != 0))
					{
						MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
							"CMashColladaLoader::CreateAnimationBuffer",
							"Only linear interpolation is supported in animations. '%s'.",
							interpolationType.GetCString());

						return 0;
					}

					sSource *sourceInput = animChannel->samplers[i]->inputSource[aANIM_INPUT_SEMANTIC_INPUT];
					sSource *sourceOutput = animChannel->samplers[i]->inputSource[aANIM_INPUT_SEMANTIC_OUTPUT];
					if (!sourceInput || !sourceOutput)
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
							"Animation source does not contain input and output data.",
							"CMashColladaLoader::CreateAnimationBuffer");

						return 0;
					}
				}
			}
		}
        
		sMashAnimationKeyTransform *transformKeys = 0;
		uint32 transformKeyArrayLength = 0;
		const f32 frameEpsilon = 0.1f;

		f32 oneOverFrameRate = 1.0f / (f32)frameRate;

		if (animChannel->samplers[aANIM_CHANNEL_MATRIX])
		{
			sSource *sourceInput = animChannel->samplers[aANIM_CHANNEL_MATRIX]->inputSource[aANIM_INPUT_SEMANTIC_INPUT];
			sSource *sourceOutput = animChannel->samplers[aANIM_CHANNEL_MATRIX]->inputSource[aANIM_INPUT_SEMANTIC_OUTPUT];

			//shouldn't happen
			if (sourceOutput->arrayData.stride != 16)
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
					"CMashColladaLoader::CreateAnimationBuffer",
					"Animation matrix data used invalid stride. '%d'.",
					sourceOutput->arrayData.stride);

				return 0;
			}

			mash::MashMatrix4 tempMatrix;
			transformKeys = (sMashAnimationKeyTransform*)m_memoryPool.GetMemory(sizeof(sMashAnimationKeyTransform) * sourceInput->arrayData.varaibleArray.count);
			const uint32 elmsToCopy = math::Clamp<uint32>(0, 16, sourceOutput->arrayData.stride);
			for(uint32 keyIndex = 0; keyIndex < sourceInput->arrayData.varaibleArray.count; ++keyIndex)
			{
				//use a small epsilon value to remove any error
				uint32 currentFrame = (sourceInput->arrayData.varaibleArray.f[keyIndex] / oneOverFrameRate) + frameEpsilon;

				transformKeys[keyIndex].frame = currentFrame;
				memcpy(tempMatrix.v, &sourceOutput->arrayData.varaibleArray.f[keyIndex * sourceOutput->arrayData.stride], sizeof(f32) * elmsToCopy);
				//transformKeys[keyIndex].frame = currentFrame;
				tempMatrix.Decompose(transformKeys[keyIndex].scaleKey, transformKeys[keyIndex].rotationKey, transformKeys[keyIndex].positionKey);
			}
		}
		else
		{
			
			{
				std::set<uint32> uniqueFrameSet;
				for(uint32 i = 0; i < aANIM_CHANNEL_COUNT; ++i)
				{
					if (animChannel->samplers[i])
					{
						sSource *sourceInput = animChannel->samplers[i]->inputSource[aANIM_INPUT_SEMANTIC_INPUT];
						if (sourceInput)
						{
							for(uint32 t = 0; t < sourceInput->arrayData.count; ++t)
							{
								//use a small epsilon value to remove any error
								uint32 currentFrame = (sourceInput->arrayData.varaibleArray.f[t] / oneOverFrameRate) + frameEpsilon;
								uniqueFrameSet.insert(currentFrame);
							}
						}
					}
				}

				transformKeyArrayLength = uniqueFrameSet.size();
				if (transformKeyArrayLength == 0)
					return 0;

				//transformKeys = MASH_ALLOC_T_COMMON(sMashAnimationKeyTransform, transformKeyArrayLength);
				transformKeys = (sMashAnimationKeyTransform*)m_memoryPool.GetMemory(sizeof(sMashAnimationKeyTransform) * transformKeyArrayLength);

				/*
					Sets are ordered from lowest to highest already, yay!
				*/
				uint32 currentFrame = 0;
				std::set<uint32>::iterator frameIter = uniqueFrameSet.begin();
				std::set<uint32>::iterator frameIterEnd = uniqueFrameSet.end();
				for(; frameIter != frameIterEnd; ++frameIter)
				{
					transformKeys[currentFrame++].frame = *frameIter;
				}
			}

			mash::MashVector3 defaultTranslation, defaultScale, defaultRotationEular;
			mash::MashQuaternion defaultRotationQuat;
			node->localTransform.Decompose(defaultScale, defaultRotationQuat, defaultTranslation);

			defaultRotationQuat.ToEulerAngles(defaultRotationEular);

			defaultRotationEular.x = math::RadsToDegs(defaultRotationEular.x);
			defaultRotationEular.y = math::RadsToDegs(defaultRotationEular.y);
			defaultRotationEular.z = math::RadsToDegs(defaultRotationEular.z);

			eANIM_CHANNEL_TARGET targets[3] = {aANIM_CHANNEL_TRANSLATION_X, aANIM_CHANNEL_TRANSLATION_Y, aANIM_CHANNEL_TRANSLATION_Z};
			for(uint32 transIndex = 0; transIndex < 3; ++transIndex)
			{
				sSource *sourceInput = 0;
				sSource *sourceOutput = 0;

				if (animChannel->samplers[targets[transIndex]])
					sourceInput = animChannel->samplers[targets[transIndex]]->inputSource[aANIM_INPUT_SEMANTIC_INPUT];
				if (animChannel->samplers[targets[transIndex]])
					sourceOutput = animChannel->samplers[targets[transIndex]]->inputSource[aANIM_INPUT_SEMANTIC_OUTPUT];

				if (!sourceInput || !sourceOutput)
				{
					for(uint32 transElmIndex = 0; transElmIndex < transformKeyArrayLength; ++transElmIndex)
					{
						transformKeys[transElmIndex].positionKey.v[transIndex] = defaultTranslation.v[transIndex];
					}
				}
				else
				{
					uint32 nextIndex = 0;
					for(uint32 transElmIndex = 0; transElmIndex < sourceInput->arrayData.varaibleArray.count; ++transElmIndex)
					{
						//use a small epsilon value to remove any error
						uint32 currentFrame = (sourceInput->arrayData.varaibleArray.f[transElmIndex] / oneOverFrameRate) + frameEpsilon;
						uint32 lastIndex = nextIndex;
						for(; nextIndex < transformKeyArrayLength; ++nextIndex)
						{
							if (transformKeys[nextIndex].frame == currentFrame)
							{
								transformKeys[nextIndex].positionKey.v[transIndex] = sourceOutput->arrayData.varaibleArray.f[transElmIndex];
								break;
							}
						}
						uint32 indexesSkipped = nextIndex - lastIndex;
						if (indexesSkipped > 1)
						{
							uint32 actualFrameDistance = currentFrame - transformKeys[lastIndex].frame;
							for(uint32 ni = 1; ni < indexesSkipped; ++ni)
							{
								f32 p = (f32)(transformKeys[nextIndex].frame - transformKeys[lastIndex + ni].frame) / (f32)actualFrameDistance;
								transformKeys[lastIndex + ni].positionKey.v[transIndex] = math::Lerp(transformKeys[lastIndex].positionKey[transIndex], transformKeys[nextIndex].positionKey[transIndex], p);
							}
						}
					}
				}
			}

			targets[0] = aANIM_CHANNEL_SCALE_X;
			targets[1] = aANIM_CHANNEL_SCALE_Y;
			targets[2] = aANIM_CHANNEL_SCALE_Z;
			for(uint32 transIndex = 0; transIndex < 3; ++transIndex)
			{
				sSource *sourceInput = 0;
				sSource *sourceOutput = 0;

				if (animChannel->samplers[targets[transIndex]])
					sourceInput = animChannel->samplers[targets[transIndex]]->inputSource[aANIM_INPUT_SEMANTIC_INPUT];
				if (animChannel->samplers[targets[transIndex]])
					sourceOutput = animChannel->samplers[targets[transIndex]]->inputSource[aANIM_INPUT_SEMANTIC_OUTPUT];

				if (!sourceInput || !sourceOutput)
				{
					for(uint32 transElmIndex = 0; transElmIndex < transformKeyArrayLength; ++transElmIndex)
						transformKeys[transElmIndex].scaleKey.v[transIndex] = defaultScale.v[transIndex];
				}
				else
				{
					uint32 nextIndex = 0;
					for(uint32 transElmIndex = 0; transElmIndex < sourceOutput->arrayData.varaibleArray.count; ++transElmIndex)
					{
						uint32 currentFrame = (sourceInput->arrayData.varaibleArray.f[transElmIndex] / oneOverFrameRate) + frameEpsilon;
						uint32 lastIndex = nextIndex;
						for(; nextIndex < transformKeyArrayLength; ++nextIndex)
						{
							if (transformKeys[nextIndex].frame == currentFrame)
							{
								transformKeys[nextIndex].scaleKey.v[transIndex] = sourceOutput->arrayData.varaibleArray.f[transElmIndex];
								break;
							}
						}
						uint32 indexesSkipped = nextIndex - lastIndex;
						if (indexesSkipped > 1)
						{
							uint32 actualFrameDistance = currentFrame - transformKeys[lastIndex].frame;
							for(uint32 ni = 1; ni < indexesSkipped; ++ni)
							{
								f32 p = (f32)(transformKeys[nextIndex].frame - transformKeys[lastIndex + ni].frame) / (f32)actualFrameDistance;
								transformKeys[lastIndex + ni].scaleKey.v[transIndex] = math::Lerp(transformKeys[lastIndex].scaleKey[transIndex], transformKeys[nextIndex].scaleKey[transIndex], p);
							}
						}

					}
				}
			}

			MashVector3 *eularAngles = (MashVector3*)m_memoryPool.GetMemory(sizeof(MashVector3) * transformKeyArrayLength);
			const f32 maxRotation = 9999999.0f;
			for(uint32 i = 0; i < transformKeyArrayLength; ++i)
			{
				eularAngles[i].x = maxRotation;
				eularAngles[i].y = maxRotation;
				eularAngles[i].z = maxRotation;
			}

			targets[0] = aANIM_CHANNEL_ROTATION_X;
			targets[1] = aANIM_CHANNEL_ROTATION_Y;
			targets[2] = aANIM_CHANNEL_ROTATION_Z;

			for(uint32 transIndex = 0; transIndex < 3; ++transIndex)
			{
				sSource *sourceInput = 0;
				sSource *sourceOutput = 0;

				if (animChannel->samplers[targets[transIndex]])
					sourceInput = animChannel->samplers[targets[transIndex]]->inputSource[aANIM_INPUT_SEMANTIC_INPUT];
				if (animChannel->samplers[targets[transIndex]])
					sourceOutput = animChannel->samplers[targets[transIndex]]->inputSource[aANIM_INPUT_SEMANTIC_OUTPUT];

				if (!sourceInput || !sourceOutput)
				{
					for(uint32 transElmIndex = 0; transElmIndex < transformKeyArrayLength; ++transElmIndex)
						eularAngles[transElmIndex].v[transIndex] = defaultRotationEular.v[transIndex];
				}
				else
				{
					uint32 nextIndex = 0;
					for(uint32 transElmIndex = 0; transElmIndex < sourceOutput->arrayData.varaibleArray.count; ++transElmIndex)
					{
						uint32 currentFrame = (sourceInput->arrayData.varaibleArray.f[transElmIndex] / oneOverFrameRate) + frameEpsilon;
						uint32 lastIndex = nextIndex;
						for(; nextIndex < transformKeyArrayLength; ++nextIndex)
						{
							if (transformKeys[nextIndex].frame == currentFrame)
							{
								eularAngles[nextIndex].v[transIndex] = sourceOutput->arrayData.varaibleArray.f[transElmIndex];
								break;
							}
						}
						uint32 indexesSkipped = nextIndex - lastIndex;
						if (indexesSkipped > 1)
						{
							uint32 actualFrameDistance = currentFrame - transformKeys[lastIndex].frame;
							for(uint32 ni = 1; ni < indexesSkipped; ++ni)
							{
								f32 p = (f32)(transformKeys[nextIndex].frame - transformKeys[lastIndex + ni].frame) / (f32)actualFrameDistance;
								eularAngles[lastIndex + ni].v[transIndex] = math::Lerp(eularAngles[lastIndex].v[transIndex], eularAngles[nextIndex].v[transIndex], p);
								eularAngles[lastIndex + ni].v[transIndex] = math::RadsToDegs(math::MapToRange(math::DegsToRads(eularAngles[lastIndex + ni].v[transIndex]), math::DegsToRads(eularAngles[lastIndex].v[transIndex])));
							}
						}
					}
				}
			}

			for(uint32 key = 0; key < transformKeyArrayLength; ++key)
			{
				transformKeys[key].rotationKey.SetEuler(-math::DegsToRads(eularAngles[key].x), 
					-math::DegsToRads(eularAngles[key].y),
					-math::DegsToRads(eularAngles[key].z));
			}

			if (eularAngles)
				m_memoryPool.FreeMemory(eularAngles);
		}

		if (transformKeys)
		{
			MashControllerManager *controllerManager = device->GetSceneManager()->GetControllerManager();
			MashAnimationBuffer *animationBuffer = controllerManager->CreateAnimationBuffer();

			MashTransformationKeySet *newTransformKeySet = controllerManager->CreateTransformationKeySet();
			newTransformKeySet->AddKeyList(transformKeys, transformKeyArrayLength);

			animationBuffer->AddAnimationKeySet("unknown", aCONTROLLER_TRANSFORMATION, newTransformKeySet);
			newTransformKeySet->Drop();//the buffer now owns the key set

			m_memoryPool.FreeMemory(transformKeys);

			return animationBuffer;
		}

		return 0;
	}

	mash::MashModel* CMashColladaLoader::CreateModel(MashDevice *device, eFILE_UP_AXIS upAxis, sMesh* model, sSkinController *skinController, const mash::MashMatrix4 &geometryOffsetTransform, const sLoadSceneSettings &loadSettings)
	{
		mash::MashVector4 *loadDataBoneWeightArray = 0;
		mash::MashVector4 *loadDataBoneIndexArray = 0;

		mash::MashVector4 *boneWeightArray = 0;
		mash::MashVector4 *boneIndexArray = 0;
		sInputData *skinControllerWeightArray = 0;
		sInputData *skinControllerIndexArray = 0;
		uint32 totalVertexWeightArrayOffset = 0;//usually 2

		if (skinController)
		{
			MashList<sInputData>::Iterator wdIter = skinController->weightData.Begin();
			MashList<sInputData>::Iterator wdIterEnd = skinController->weightData.End();
			for(; wdIter != wdIterEnd; ++wdIter)
			{
				if (wdIter->usage == aEXT_INPUT_SEMANTIC_JOINT)
					skinControllerIndexArray = &(*wdIter);
				else if (wdIter->usage == aEXT_INPUT_SEMANTIC_WEIGHT)
					skinControllerWeightArray = &(*wdIter);

				if (wdIter->indexOffset > totalVertexWeightArrayOffset)
					totalVertexWeightArrayOffset = wdIter->indexOffset;
			}

			totalVertexWeightArrayOffset += 1;
		}

		sMesh *currentMesh = model;
		while(currentMesh->prevLod)
			currentMesh = currentMesh->prevLod;

		mash::MashModel *engineModel = device->GetSceneManager()->CreateModel();
		//loop through each lod
		do
		{
			MashArray<mash::MashMesh*> meshLods;
			for(uint32 i = 0; i < currentMesh->meshCount; ++i)
			{
				sMashVertexElement *vertexElements = (sMashVertexElement*)m_memoryPool.GetMemory(sizeof(sMashVertexElement) * currentMesh->meshArray[i].vertexLayout.Size());
				uint32 vertexStrideInBytes = 0;
				uint32 triangleDataOffset = 0;

				uint32 currentElement = 0;
				uint32 usageIndex[aVERTEX_DECLUSAGE_COUNT];
				memset(usageIndex, 0, sizeof(usageIndex));
				/*
					Assume POSITION element is first
				*/
				uint32 currentVertexElementStride = 0;
				MashList<sInputData>::Iterator vertexLayoutIter = currentMesh->meshArray[i].vertexLayout.Begin();
				MashList<sInputData>::Iterator vertexLayoutIterEnd = currentMesh->meshArray[i].vertexLayout.End();
				for(; vertexLayoutIter != vertexLayoutIterEnd; ++vertexLayoutIter)
				{
					sArrayDataSource *arrayData = &vertexLayoutIter->sourceArray->arrayData;

					switch(vertexLayoutIter->usage)
					{
					case aDECLUSAGE_POSITION:
					case aDECLUSAGE_NORMAL:
						{
							ConvertVectorArrayAxisIfNeeded(upAxis, arrayData);
							break;
						}
					case aDECLUSAGE_TEXCOORD:
					case aDECLUSAGE_TANGENT:
						{
							ConvertTexCoordArrayAxisIfNeeded(arrayData);
							break;
						}
					}
						

					uint32 dataArrayTypeSizeInBytes = 0;
					switch(arrayData->varaibleArray.type)
					{
					case aARRAY_DATA_SOURCE_FLOAT:
						dataArrayTypeSizeInBytes = sizeof(f32);
						break;
					case aARRAY_DATA_SOURCE_INT:
						dataArrayTypeSizeInBytes = sizeof(int32);
						break;
					case aARRAY_DATA_SOURCE_BOOL:
						dataArrayTypeSizeInBytes = sizeof(bool);
						break;
					default:
						MASH_ASSERT(0);
					}

					vertexStrideInBytes += arrayData->stride * dataArrayTypeSizeInBytes;

					if (vertexLayoutIter->indexOffset > triangleDataOffset)
						triangleDataOffset = vertexLayoutIter->indexOffset;

					vertexElements[currentElement].stream = 0;

					//shouldn't happen
					MASH_ASSERT(vertexLayoutIter->usage < aVERTEX_DECLUSAGE_COUNT);
					
					switch(arrayData->stride)
					{
					case 1:
						vertexElements[currentElement].type = aDECLTYPE_R32_FLOAT;
						break;
					case 2:
						vertexElements[currentElement].type = aDECLTYPE_R32G32_FLOAT;
						break;
					case 3:
						vertexElements[currentElement].type = aDECLTYPE_R32G32B32_FLOAT;
						break;
					case 4:
						vertexElements[currentElement].type = aDECLTYPE_R32G32B32A32_FLOAT;
						break;
					default:
						MASH_ASSERT(0);
					}

					vertexElements[currentElement].stride = currentVertexElementStride;
					currentVertexElementStride += mash::helpers::GetVertexTypeSize(vertexElements[currentElement].type);

					vertexElements[currentElement].usage = (eVERTEX_DECLUSAGE)vertexLayoutIter->usage;
					vertexElements[currentElement].usageIndex = usageIndex[vertexElements[currentElement].usage]++;
					vertexElements[currentElement].classification = aCLASSIFICATION_VERTEX_DATA;
					vertexElements[currentElement].instanceStepRate = 0;

					++currentElement;
				}

				/*
					Advance the index count by one so the correct array index is calculated
				*/
				++triangleDataOffset;

				const uint32 vertexCount = currentMesh->meshArray[i].triangleCount * 3;
				
				/*
					TODO : Keep this array around for the whole model
				*/
				uint8 *vertexBuffer = (uint8*)m_memoryPool.GetMemory(vertexStrideInBytes * vertexCount);

				if (skinController)
				{
					uint32 arrayCount = currentMesh->meshArray[i].triangleCount * 3;
					boneWeightArray = (MashVector4*)m_memoryPool.GetMemory(sizeof(MashVector4) * arrayCount);
					boneIndexArray = (MashVector4*)m_memoryPool.GetMemory(sizeof(MashVector4) * arrayCount);

					for(uint32 i = 0; i < arrayCount; ++i)
					{
						boneWeightArray[i].Zero();
						boneIndexArray[i].Zero();
					}

					loadDataBoneWeightArray = (MashVector4*)m_memoryPool.GetMemory(sizeof(MashVector4) * skinController->vertexWeightCount);
					loadDataBoneIndexArray = (MashVector4*)m_memoryPool.GetMemory(sizeof(MashVector4) * skinController->vertexWeightCount);

					uint32 vertexBoneWeightDataIndex = 0;
					for(uint32 i = 0; i < skinController->vertexWeightCount; ++i)
					{
						int32 originalVertexBoneCount = skinController->vertexBoneCounts.i[i];

						if (originalVertexBoneCount > 4)
						{
							MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
								"CMashColladaLoader::CreateModel",
								"Vertex bone influence is limited to 4. A value of '%d' was found.",
								originalVertexBoneCount);
						}

						int32 vertexBoneCount = math::Clamp<int32>(0, 4, originalVertexBoneCount);
						uint32 b = 0;
						for(; b < vertexBoneCount; ++b)
						{
							uint32 wi = skinController->vertexBoneWeightIndices.i[(vertexBoneWeightDataIndex * totalVertexWeightArrayOffset) + (b * totalVertexWeightArrayOffset) + skinControllerWeightArray->indexOffset];
							int32 ji = skinController->vertexBoneWeightIndices.i[(vertexBoneWeightDataIndex * totalVertexWeightArrayOffset) + (b * totalVertexWeightArrayOffset)  + skinControllerIndexArray->indexOffset];

							loadDataBoneWeightArray[i].v[b] = skinControllerWeightArray->sourceArray->arrayData.varaibleArray.f[wi];
							loadDataBoneIndexArray[i].v[b] = (f32)ji;
						}

						for(; b < 4; ++b)
						{
							loadDataBoneWeightArray[i].v[b] = 0.0f;
							loadDataBoneIndexArray[i].v[b] = 0.0f;
						}
						
						
						/*
							TODO : Normalize vertex weights. Dont use the vector::Normalize method.
							Normalize by vertexBoneCount
						*/
						/*if (vertexBoneCount > 0)
						{
							for(; b < vertexBoneCount; ++b)
							{
								loadDataBoneWeightArray[i].v[b] /= weightTotal;
							}
						}*/

						//loadDataBoneWeightArray[i].Normalize();

						vertexBoneWeightDataIndex += originalVertexBoneCount;
					}
				}

				uint32 newEngineVertIndex = 0;
				uint32 flippedFaceVert = 0;
				for(uint32 primIndex = 0; primIndex < currentMesh->meshArray[i].triangleCount; ++primIndex)
				{
					for(uint32 faceVert = 0; faceVert < 3; ++faceVert, ++newEngineVertIndex)
					{
						//Here we flip the vertex winding order. It is currently CCW, we need it as CW.
						flippedFaceVert = faceVert;
						if (flippedFaceVert == 1)
							flippedFaceVert = 2;
						else if (flippedFaceVert == 2)
							flippedFaceVert = 1;

						uint32 vertexElementOffset = 0;

						vertexLayoutIter = currentMesh->meshArray[i].vertexLayout.Begin();
						vertexLayoutIterEnd = currentMesh->meshArray[i].vertexLayout.End();
						for(; vertexLayoutIter != vertexLayoutIterEnd; ++vertexLayoutIter)
						{
							sArrayDataSource *arrayData = &vertexLayoutIter->sourceArray->arrayData;

							int32 arrayIndexStart = currentMesh->meshArray[i].triangleIndices.i[(((primIndex * 3) + flippedFaceVert) * triangleDataOffset) + vertexLayoutIter->indexOffset] * arrayData->stride;
							arrayIndexStart += arrayData->offset;

							if ((vertexLayoutIter->usage == aDECLUSAGE_POSITION))
							{
								const uint32 elementsToCopy = math::Clamp<uint32>(0, 3, arrayData->stride);
								mash::MashVector3 pos;
								memcpy(pos.v, &arrayData->varaibleArray.f[arrayIndexStart], sizeof(f32) * elementsToCopy);

								pos = geometryOffsetTransform.TransformVector(pos);
								memcpy(&vertexBuffer[(newEngineVertIndex * vertexStrideInBytes) + vertexElementOffset], pos.v, sizeof(f32) * elementsToCopy);
								vertexElementOffset += sizeof(f32) * arrayData->stride;
							}
							else
							{
								eARRAY_DATA_SOURCE_TYPE arrayType = arrayData->varaibleArray.type;
								switch(arrayType)
								{
								case aARRAY_DATA_SOURCE_FLOAT:
									memcpy(&vertexBuffer[(newEngineVertIndex * vertexStrideInBytes) + vertexElementOffset], &arrayData->varaibleArray.f[arrayIndexStart], sizeof(f32) * arrayData->stride);
									vertexElementOffset += sizeof(f32) * arrayData->stride;
									break;
								case aARRAY_DATA_SOURCE_INT:
									memcpy(&vertexBuffer[(newEngineVertIndex * vertexStrideInBytes) + vertexElementOffset], &arrayData->varaibleArray.i[arrayIndexStart], sizeof(int32) * arrayData->stride);
									vertexElementOffset += sizeof(int32) * arrayData->stride;
									break;
								case aARRAY_DATA_SOURCE_BOOL:
									memcpy(&vertexBuffer[(newEngineVertIndex * vertexStrideInBytes) + vertexElementOffset], &arrayData->varaibleArray.b[arrayIndexStart], sizeof(bool) * arrayData->stride);
									vertexElementOffset += sizeof(bool) * arrayData->stride;
									break;
								default:
									MASH_ASSERT(0);
								}
							}
						}

						if (skinController)
						{
							int32 vcountArrayIndex = currentMesh->meshArray[i].triangleIndices.i[((primIndex * 3) + flippedFaceVert) * triangleDataOffset];
							boneWeightArray[newEngineVertIndex] = loadDataBoneWeightArray[vcountArrayIndex];
							boneIndexArray[newEngineVertIndex] = loadDataBoneIndexArray[vcountArrayIndex];
						}
					}
				}

				uint32 *indices = (uint32*)m_memoryPool.GetMemory(sizeof(uint32) * vertexCount);
				for(uint32 idx = 0; idx < vertexCount; ++idx)
					indices[idx] = idx;

				MashMeshBuilder::sMesh newMesh;
				newMesh.vertices = vertexBuffer;
				newMesh.vertexCount = vertexCount;
				newMesh.indices = (uint8*)indices;
				newMesh.indexCount = vertexCount;
				newMesh.indexFormat = aFORMAT_R32_UINT;
				newMesh.currentVertexElements = vertexElements;
				newMesh.currentVertexElementCount = currentMesh->meshArray[i].vertexLayout.Size();
				newMesh.boneWeightArray = boneWeightArray;
				newMesh.boneIndexArray = boneIndexArray;
				newMesh.primitiveType = aPRIMITIVE_TRIANGLE_LIST;

				MashMaterial *material = 0;
				if (model->meshArray[i].materialName)
					material = device->GetRenderer()->GetMaterialManager()->FindMaterial(currentMesh->meshArray[i].materialName);

				if (!material)
					material = device->GetRenderer()->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_DEFAULT_MESH);

				if (!material)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
							"Failed to load material for mesh.",
							"CMashColladaLoader::CreateModel");

					return 0;
				}

				mash::MashMesh *engineMesh = device->GetSceneManager()->CreateStaticMesh();

				uint32 meshFlags = MashMeshBuilder::aMESH_UPDATE_FILL_MESH | 
					MashMeshBuilder::aMESH_UPDATE_CHANGE_VERTEX_FORMAT | 
					MashMeshBuilder::aMESH_UPDATE_WELD;

				if (newMesh.indexCount < 65535)
					meshFlags |= MashMeshBuilder::aMESH_UPDATE_16BIT_INDICES;

				if (device->GetSceneManager()->GetMeshBuilder()->UpdateMeshEx(engineMesh, &newMesh, material->GetVertexDeclaration(), meshFlags) == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to covert mesh into the materials vertex format.", 
						"CMashColladaLoader::CreateModel");

					return 0;
				}

				if (vertexElements)
				{
					m_memoryPool.FreeMemory(vertexElements);
					vertexElements = 0;
				}

				if (vertexBuffer)
				{
					m_memoryPool.FreeMemory(vertexBuffer);
					vertexBuffer = 0;
				}

				if (indices)
				{
					m_memoryPool.FreeMemory(indices);
					indices = 0;
				}
				
				if (boneWeightArray)
					m_memoryPool.FreeMemory(boneWeightArray);

				if (boneIndexArray)
					m_memoryPool.FreeMemory(boneIndexArray);

				if (loadDataBoneWeightArray)
					m_memoryPool.FreeMemory(loadDataBoneWeightArray);

				if (loadDataBoneIndexArray)
					m_memoryPool.FreeMemory(loadDataBoneIndexArray);

				/*
					TODO : If the mesh is skinned then create the L,W,H of the AABB to be
					as large as the max dimention.
					It should be noted in the engine documentation that in order for a skinned characters
					AABB to contain the entire mesh during animation, the bind pose should be the max
					extent of the mesh.
				*/
				mash::MashAABB boundingBox;
				if (device->GetSceneManager()->GetMeshBuilder()->CalculateBoundingBox(engineMesh->GetVertexDeclaration(), 
					engineMesh->GetRawVertices().Pointer(), engineMesh->GetVertexCount(), boundingBox) == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to create bounding box for mesh.",
						"CMashColladaLoader::CreateModel");

					return 0;
				}

				engineMesh->SetBoundingBox(boundingBox);

				meshLods.PushBack(engineMesh);

				engineMesh->SetSaveInitialiseDataFlags(loadSettings.saveGeometryFlags);
				if (loadSettings.deleteMeshInitialiseDataOnLoad)
				{
					engineMesh->DeleteInitialiseData();
				}
			}

			if (!meshLods.Empty())
			{
				engineModel->Append(&meshLods[0], meshLods.Size());

				for(uint32 m = 0; m < meshLods.Size(); ++m)
					meshLods[m]->Drop();

				meshLods.Clear();
			}

		}while(currentMesh = currentMesh->nextLod);

		return engineModel;
	}

	eMASH_STATUS CMashColladaLoader::ReadMeshData(MashXMLReader *xmlReader, sMesh *newMesh, const int8 *siblingName, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap, MashList<sInputData> &vertexInputs)
	{
		do
		{
			newMesh->meshArray = (sSubMesh*)m_memoryPool.GetMemory(sizeof(sSubMesh) * newMesh->meshCount);
			MashMemoryAllocatorHelper<sSubMesh>::ConstructArray(newMesh->meshArray, newMesh->meshCount);
			uint32 currentMeshIndex = 0;
			sSubMesh *currentMesh = &newMesh->meshArray[currentMeshIndex];

			currentMesh->materialName = xmlReader->GetAttributeRaw("material");

			//this is the stride in the index array per vertex
			uint32 totalOffsetPerVertex = 0;
			currentMesh->triangleCount = 0;
			xmlReader->GetAttributeInt("count", currentMesh->triangleCount);
			if (xmlReader->MoveToFirstChild("input"))
			{
				do
				{
					GetInputSemanticData(xmlReader, sourceMap, currentMesh->vertexLayout, &vertexInputs);

					if (!currentMesh->vertexLayout.Empty() && (currentMesh->vertexLayout.Back().indexOffset > totalOffsetPerVertex))
						totalOffsetPerVertex = currentMesh->vertexLayout.Back().indexOffset;

				}while(xmlReader->MoveToNextSibling("input"));

				
				//indices
				if (xmlReader->MoveToNextSibling("p"))
				{
					currentMesh->triangleIndices.count = currentMesh->triangleCount * (totalOffsetPerVertex + 1) * 3;
					currentMesh->triangleIndices.type = aARRAY_DATA_SOURCE_INT;
					currentMesh->triangleIndices.i = (int32*)m_memoryPool.GetMemory(sizeof(int32) * currentMesh->triangleIndices.count);

					uint32 currentOffset = 0;
					do
					{
						uint32 elementsWritten = 0;
						ConvertTextArrayToElements(xmlReader->GetTextRaw(), currentMesh->triangleIndices, currentOffset, &elementsWritten);
						currentOffset += elementsWritten;
					}while(xmlReader->MoveToNextSibling("p"));

					if (((currentOffset / (totalOffsetPerVertex + 1)) % 3) != 0)
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
							"Loaded mesh does not appear to be made up of triangles.",
							"CMashColladaLoader::ReadMeshData");

						return aMASH_FAILED;
					}
				}
				
				xmlReader->PopChild();
			}

		}while(xmlReader->MoveToNextSibling(siblingName));

		return aMASH_OK;
	}

	void CMashColladaLoader::ReadAnimationData(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis,  std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc > &animSamplerMap, std::map<MashStringc, sAnimChannel, std::less<MashStringc>, animChannelAlloc > &animChannelMap, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap)
	{
		do
		{
			if (xmlReader->MoveToFirstChild("source"))
			{
				do
				{
					GetSourceElements(xmlReader, sourceMap);
				}while(xmlReader->MoveToNextSibling("source"));
				xmlReader->PopChild();
			}

			if (xmlReader->MoveToFirstChild("sampler"))
			{
				do
				{
					GetAnimSamplerData(xmlReader, sourceMap, animSamplerMap);
				}while(xmlReader->MoveToNextSibling("sampler"));
				xmlReader->PopChild();
			}

			if (xmlReader->MoveToFirstChild("channel"))
			{
				do
				{
					GetAnimChannelData(xmlReader, upAxis, animSamplerMap, animChannelMap);
				}while(xmlReader->MoveToNextSibling("channel"));
				xmlReader->PopChild();
			}

			if (xmlReader->MoveToFirstChild("animation"))
			{
				ReadAnimationData(xmlReader, upAxis, animSamplerMap, animChannelMap, sourceMap);
				xmlReader->PopChild();
			}
		}while(xmlReader->MoveToNextSibling("animation"));
	}

	void CMashColladaLoader::ReadLibraryGeometry(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, std::map<MashStringc, sMesh*, std::less<MashStringc>, meshAlloc > &modelMap, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap)
	{
		std::less<MashStringc> cmp;
		std::map<MashStringc, sMesh*, std::less<MashStringc>, meshAlloc > lodModelMap(cmp, meshAlloc(&m_memoryPool));

		if (xmlReader->MoveToFirstChild("geometry"))
		{
			do
			{
				const int8 *modelFileId = xmlReader->GetAttributeRaw("id");
				const int8 *modelName = xmlReader->GetAttributeRaw("name");

				if (xmlReader->MoveToFirstChild("mesh"))
				{
					MashList<sInputData> vertexInputs;
					sMesh *newMesh = (sMesh*)m_memoryPool.GetMemory(sizeof(sMesh));
					MashMemoryAllocatorHelper<sMesh>::Construct(newMesh);
					newMesh->meshCount = 0;
					newMesh->nextLod = 0;
					newMesh->prevLod = 0;
					newMesh->engineModel = 0;

					if (xmlReader->MoveToFirstChild("source"))
					{
						do
						{
							GetSourceElements(xmlReader, sourceMap);

						}while(xmlReader->MoveToNextSibling("source"));

						xmlReader->PopChild();
					}

					if (xmlReader->MoveToFirstChild("vertices"))
					{
						do
						{
							if (xmlReader->MoveToFirstChild("input"))
							{
								//usually only POSITION is found here
								do
								{
									GetInputSemanticData(xmlReader, sourceMap, vertexInputs, 0);

								}while(xmlReader->MoveToNextSibling());
								
								xmlReader->PopChild();
							}

						}while(xmlReader->MoveToNextSibling("vertices"));

						xmlReader->PopChild();
					}

					/*
						Meshes in a perfect world would just come in as triangles.
						But unfortunatly some exporters will state polygons even
						though they were exported as triangles.
					*/
					if (xmlReader->MoveToFirstChild("triangles"))
					{
						do
						{
							++newMesh->meshCount;
						}while(xmlReader->MoveToNextSibling("triangles"));

						xmlReader->PopChild();
					}
					else if (xmlReader->MoveToFirstChild("polygons"))
					{
						do
						{
							++newMesh->meshCount;
						}while(xmlReader->MoveToNextSibling("polygons"));

						xmlReader->PopChild();
					}

					if (newMesh->meshCount > 0)
					{
						if (xmlReader->MoveToFirstChild("triangles"))
						{
							do
							{
								if (ReadMeshData(xmlReader, newMesh, "triangles", sourceMap, vertexInputs) == aMASH_FAILED)
								{
									newMesh->meshCount = 0;
									break;
								}

							}while(xmlReader->MoveToNextSibling("triangles"));

							xmlReader->PopChild();
						}
						else if (xmlReader->MoveToFirstChild("polygons"))
						{
							do
							{
								if (ReadMeshData(xmlReader, newMesh, "polygons", sourceMap, vertexInputs) == aMASH_FAILED)
								{
									newMesh->meshCount = 0;
									break;
								}

							}while(xmlReader->MoveToNextSibling("polygons"));

							xmlReader->PopChild();
						}
					}

					if (newMesh->meshCount > 0)
					{
						int32 nameFlags;
						DecodeName(modelName, newMesh->meshName, nameFlags, newMesh->lodIndex);

						newMesh->id = modelFileId;

						/*
							If it's a lod model then we store this in a seperate list. It will be
							added to the model list after it has been sorted
						*/
						if (!(nameFlags & aNAME_FLAG_LOD))
							modelMap.insert(std::make_pair(modelFileId, newMesh));
						else
						{
							std::map<MashStringc, sMesh*, std::less<MashStringc>, meshAlloc >::iterator mIter = lodModelMap.find(newMesh->meshName);
							if (mIter == lodModelMap.end())
							{
								lodModelMap.insert(std::make_pair(newMesh->meshName, newMesh));
							}
							else
							{
								sMesh *n = mIter->second;
								while(n->nextLod)
									n = n->nextLod;

								n->nextLod = newMesh;
							}
						}
					}
					else
					{
						//some error occured
						MASH_DELETE_T(sMesh, newMesh);
					}

					xmlReader->PopChild();
				}

			}while(xmlReader->MoveToNextSibling("geometry"));

			xmlReader->PopChild();
		}

		/*
			Sort mesh lods.

			Note, models with lods > 0 will not be stored by name in the model list. Lods > 0 are 
			collapesed into lod 0. Therefore any references to lods > 0 will not be found later.
		*/
		std::map<MashStringc, sMesh*, std::less<MashStringc>, meshAlloc >::iterator mIter = lodModelMap.begin();
		std::map<MashStringc, sMesh*, std::less<MashStringc>, meshAlloc >::iterator mIterEnd = lodModelMap.end();
		for(; mIter != lodModelMap.end(); ++mIter)
		{
			while(true)
			{
				sMesh *meshListHead = mIter->second;
				while(meshListHead->prevLod)
					meshListHead = meshListHead->prevLod;
				
				bool swapped = false;
				sMesh *currentNode = meshListHead;
				do
				{
					if (currentNode->nextLod && (currentNode->lodIndex > currentNode->nextLod->lodIndex))
					{
						swapped = true;

						if (currentNode->prevLod)
						{
							//remove node
							currentNode->prevLod->nextLod = currentNode->nextLod;
							currentNode->nextLod->prevLod = currentNode->prevLod;
						}

						//insert node
						sMesh *insertStart = currentNode->nextLod;
						sMesh *insertEnd = currentNode->nextLod->nextLod;

						insertStart->nextLod = currentNode;
						currentNode->prevLod = insertStart;

						currentNode->nextLod = insertEnd;
						if (insertEnd)
							insertEnd->prevLod = currentNode;
					}
				}while(currentNode = currentNode->nextLod);

				/*
					If elements were not swapped then the list is in order.
					We add the head of the list to the model map and continue
					onto the next lod list.
				*/
				if (!swapped)
				{
					modelMap.insert(std::make_pair(meshListHead->id, meshListHead));
					break;
				}
			}
		}
	}

	void CMashColladaLoader::ReadLibraryControllers(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, MashDevice *device, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap, std::map<MashStringc, sSkinController, std::less<MashStringc>, skinControllerAlloc > &skinControllers)
	{
		if (xmlReader->MoveToFirstChild("controller"))
		{
			do
			{
				const int8 *controllerId = xmlReader->GetAttributeRaw("id");

				if (xmlReader->MoveToFirstChild("skin"))
				{
					do
					{
						sSkinController newSkinController;
						newSkinController.id = controllerId;
						newSkinController.geomOwner = RemoveStringHash(xmlReader->GetAttributeRaw("source"));
						newSkinController.engineSkin = device->GetSceneManager()->CreateSkin();

						if (xmlReader->MoveToFirstChild("bind_shape_matrix"))
						{
							GetMatrixFromArray(xmlReader->GetTextRaw(), upAxis, newSkinController.bindShapeMatrix);
							xmlReader->PopChild();
						}

						if (xmlReader->MoveToFirstChild("source"))
						{
							do
							{
								GetSourceElements(xmlReader, sourceMap);

							}while(xmlReader->MoveToNextSibling("source"));

							xmlReader->PopChild();
						}

						if (xmlReader->MoveToFirstChild("joints"))
						{
							do
							{
								if (xmlReader->MoveToFirstChild("input"))
								{
									do
									{
										GetInputSemanticData(xmlReader, sourceMap, newSkinController.jointData, 0);

									}while(xmlReader->MoveToNextSibling("input"));
									
									xmlReader->PopChild();
								}
							}while(xmlReader->MoveToNextSibling("joints"));
							xmlReader->PopChild();
						}

						newSkinController.vertexWeightCount = 0;
						if (xmlReader->MoveToFirstChild("vertex_weights"))
						{
							xmlReader->GetAttributeInt("count", newSkinController.vertexWeightCount);

							if (newSkinController.vertexWeightCount > 0 && xmlReader->MoveToFirstChild("input"))
							{
								do
								{
									GetInputSemanticData(xmlReader, sourceMap, newSkinController.weightData, 0);

								}while(xmlReader->MoveToNextSibling("input"));

								if (xmlReader->MoveToNextSibling("vcount"))
								{
									newSkinController.vertexBoneCounts.count = newSkinController.vertexWeightCount;
									newSkinController.vertexBoneCounts.type = aARRAY_DATA_SOURCE_INT;
									newSkinController.vertexBoneCounts.i = (int32*)m_memoryPool.GetMemory(sizeof(int32) * newSkinController.vertexBoneCounts.count);
									ConvertTextArrayToElements(xmlReader->GetTextRaw(), newSkinController.vertexBoneCounts);
								}

								if (xmlReader->MoveToNextSibling("v"))
								{
									uint32 vArrayLength = 0;
									uint32 jointDataCount = newSkinController.jointData.Size();
									for(uint32 bc = 0; bc < newSkinController.vertexBoneCounts.count; ++bc)
										vArrayLength += newSkinController.vertexBoneCounts.i[bc] * jointDataCount;

									newSkinController.vertexBoneWeightIndices.count = vArrayLength;
									newSkinController.vertexBoneWeightIndices.type = aARRAY_DATA_SOURCE_INT;
									newSkinController.vertexBoneWeightIndices.i = (int32*)m_memoryPool.GetMemory(sizeof(int32) * newSkinController.vertexBoneWeightIndices.count);
									ConvertTextArrayToElements(xmlReader->GetTextRaw(), newSkinController.vertexBoneWeightIndices);
								}
								
								xmlReader->PopChild();
							}
							xmlReader->PopChild();
						}

						//insert the skin controller into the map
						skinControllers[newSkinController.id] = newSkinController;

						xmlReader->PopChild();
					}while(xmlReader->MoveToNextSibling("skin"));
				}
			}while(xmlReader->MoveToNextSibling());

			xmlReader->PopChild();
		}
	}

	void CMashColladaLoader::ReadLibraryVisualScene(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, MashList<sNode> &nodes)
	{
		if (xmlReader->MoveToFirstChild("visual_scene"))
		{
			if (xmlReader->MoveToFirstChild("node"))
			{
				GetNodeData(xmlReader, upAxis, 0, nodes);

				xmlReader->PopChild();
			}
			xmlReader->PopChild();
		}
	}

	void CMashColladaLoader::ReadLibraryAnimation(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap, std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc > &animSamplerMap, std::map<MashStringc, sAnimChannel, std::less<MashStringc>, animChannelAlloc > &animChannelMap)
	{
		if (xmlReader->MoveToFirstChild("animation"))
		{
			ReadAnimationData(xmlReader, upAxis, animSamplerMap, animChannelMap, sourceMap);

			xmlReader->PopChild();
		}
	}

	void CMashColladaLoader::ReadLibraryLights(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, std::map<MashStringc, sLightData, std::less<MashStringc>, lightAlloc > &lightMap)
	{
		if (xmlReader->MoveToFirstChild("light"))
		{
			do
			{
				const int8 *lightId = xmlReader->GetAttributeRaw("id");
				if (lightId)
				{
					if (xmlReader->MoveToFirstChild("technique_common"))
					{
						if (xmlReader->MoveToFirstChild())
						{
							if (strcmp(xmlReader->GetNameRaw(), "point") == 0)
							{
								lightMap[lightId].lightType = aLIGHT_POINT;
							}
							else if (strcmp(xmlReader->GetNameRaw(), "directional") == 0)
							{
								lightMap[lightId].lightType = aLIGHT_DIRECTIONAL;
							}
							else if (strcmp(xmlReader->GetNameRaw(), "spot") == 0)
							{
								lightMap[lightId].lightType = aLIGHT_SPOT;
							}

							xmlReader->PopChild();	
						}
						xmlReader->PopChild();	
					}
				}
			}while(xmlReader->MoveToNextSibling("light"));
			
			xmlReader->PopChild();
		}
	}

	eMASH_STATUS CMashColladaLoader::Load(MashDevice *device, const MashStringc &filename, MashList<mash::MashSceneNode*> &rootNodes, const sLoadSceneSettings &loadSettings)
	{
		//clear any previously loaded data
		m_memoryPool.Clear();

		MashXMLReader *xmlReader = device->GetFileManager()->CreateXMLReader(filename.GetCString());

		if (!xmlReader)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
						"CMashColladaLoader::Load",
						"Collada file not found.",
						filename.GetCString());

			return aMASH_FAILED;
		}

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
					"CMashColladaLoader::Load",
					"Started to load Collada file '%s'.",
					filename.GetCString());

		sFileData fileData;
		MashStringc stringBuffer;
		
		if (xmlReader->MoveToFirstChild("asset"))
		{
			if (xmlReader->MoveToFirstChild("up_axis"))
			{
				stringBuffer.Clear();
				xmlReader->GetText(stringBuffer);

				if (strcmp(stringBuffer.GetCString(), "Z_UP") == 0)
					fileData.upAxis = aFILE_UP_AXIS_Z;
				else if (strcmp(stringBuffer.GetCString(), "Y_UP") == 0)
					fileData.upAxis = aFILE_UP_AXIS_Y;
				else
					fileData.upAxis = aFILE_UP_AXIS_X;

				xmlReader->PopChild();
			}
		}

		std::less<MashStringc> cmp;
		std::map<MashStringc, sMesh*, std::less<MashStringc>, meshAlloc > modelMap(cmp, meshAlloc(&m_memoryPool));
		std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > sourceMap(cmp, sourceAlloc(&m_memoryPool));

		std::map<MashStringc, sSkinController, std::less<MashStringc>, skinControllerAlloc > skinControllers(cmp, skinControllerAlloc(&m_memoryPool));

		MashList<sNode> nodes;

		std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc > animSamplerMap(cmp, animSampleAlloc(&m_memoryPool));
		std::map<MashStringc, sAnimChannel, std::less<MashStringc>, animChannelAlloc > animChannelMap(cmp, animChannelAlloc(&m_memoryPool));

		std::map<MashStringc, sLightData, std::less<MashStringc>, lightAlloc > lightMap(cmp, lightAlloc(&m_memoryPool));
		if (xmlReader->MoveToNextSibling())
		{
			do
			{
				if (strcmp(xmlReader->GetNameRaw(), "library_geometries") == 0)
				{
					ReadLibraryGeometry(xmlReader, fileData.upAxis, modelMap, sourceMap);
				}
				else if (strcmp(xmlReader->GetNameRaw(), "library_controllers") == 0)
				{
					ReadLibraryControllers(xmlReader, fileData.upAxis, device, sourceMap, skinControllers);
				}
				else if (strcmp(xmlReader->GetNameRaw(), "library_visual_scenes") == 0)
				{
					ReadLibraryVisualScene(xmlReader, fileData.upAxis, nodes);
				}
				else if (strcmp(xmlReader->GetNameRaw(), "library_animations") == 0)
				{
					ReadLibraryAnimation(xmlReader, fileData.upAxis, sourceMap, animSamplerMap, animChannelMap);
				}
				else if (strcmp(xmlReader->GetNameRaw(), "library_lights") == 0)
				{
					ReadLibraryLights(xmlReader, fileData.upAxis, lightMap);
				}

			}while(xmlReader->MoveToNextSibling());
		}

		mash::MashDummy *rootNode = 0;

		if (loadSettings.createRootNode)
		{
			/*
				Get file name, minus the extension
			*/
			MashStringc rootNodeName = "";
            GetFileName(filename.GetCString(), rootNodeName);
			rootNode = device->GetSceneManager()->AddDummy(0, rootNodeName.GetCString());

			rootNodes.PushBack(rootNode);
		}

		typedef MashList<sNode>::Iterator tdNodeListIter;
		MashArray<tdNodeListIter> nodesContainingControllers;

		/*
			Convert data to engine objects
		*/
		MashStringc nameBuffer;
		tdNodeListIter nodeIter = nodes.Begin();
		tdNodeListIter nodeIterEnd = nodes.End();
		for(; nodeIter != nodeIterEnd; ++nodeIter)
		{
			if (!nodeIter->decodedName.Empty())
				nameBuffer = nodeIter->decodedName;
			else
				device->GetSceneManager()->GenerateUniqueSceneNodeName(nameBuffer);

			if (nodeIter->nodeType == aCOLLADA_NODE_TYPE_JOINT)
			{
				//create bone
				MashBone *bone = device->GetSceneManager()->AddBone(0, nameBuffer);
				nodeIter->engineNode = bone;
			}
			else
			{
				sSkinController *skinController = 0;
				if (nodeIter->controllerName)
				{
					std::map<MashStringc, sSkinController, std::less<MashStringc>, skinControllerAlloc >::iterator skinIter = skinControllers.find(nodeIter->controllerName);
					if (skinIter != skinControllers.end())
						skinController = &skinIter->second;

					/*
						skin controllers contain the geometry name
					*/
					if (skinController && skinController->geomOwner)
						nodeIter->geometryName = skinController->geomOwner;
				}

				if (nodeIter->geometryName)
				{
					std::map<MashStringc, sMesh*, std::less<MashStringc>, meshAlloc >::iterator mIter = modelMap.find(nodeIter->geometryName);
					/*
						Models with lods > 0 will not be stored by name in this list. Lods > 0 are 
						collapesed into lod 0. Therefore nodes that reference a model with lod > 0
						will not be created
					*/
					if (mIter != modelMap.end())
					{
						MashModel *model = mIter->second->engineModel;
						if (!model)
						{
							mash::MashMatrix4 geometryOffsetTransform;
							if (skinController)//if it has a skin then both these matrices should be the same
								geometryOffsetTransform = nodeIter->geometryOffsetTransform;
							else
								geometryOffsetTransform = nodeIter->geometryOffsetTransform;

							model = CreateModel(device, fileData.upAxis, mIter->second, skinController, geometryOffsetTransform, loadSettings);
							mIter->second->engineModel = model;
						}

						if (model)
						{
							MashEntity *entity = device->GetSceneManager()->AddEntity(0, model, nameBuffer);
							nodeIter->engineNode = entity;

							entity->SetModel(model);

							if (skinController)
							{
								entity->SetSkin(skinController->engineSkin);
								nodesContainingControllers.PushBack(nodeIter);
							}

							//make sure we are at the head of the list
							sMesh *meshHead = mIter->second;
							while(meshHead->prevLod)
								meshHead = meshHead->prevLod;

							uint32 lodIndex = 0;
							do
							{
								for(uint32 subMeshIndex = 0; subMeshIndex < meshHead->meshCount; ++subMeshIndex)
								{
									MashMaterial *material = 0;
									if (meshHead->meshArray[subMeshIndex].materialName)
										material = device->GetRenderer()->GetMaterialManager()->FindMaterial(meshHead->meshArray[subMeshIndex].materialName);

									if (!material)
										material = device->GetRenderer()->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_DEFAULT_MESH);

									if (!material)
									{
										MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
												"Failed to load material for mesh.",
												"CMashColladaLoader::Load");

										//TODO : Break better
										return aMASH_FAILED;
									}

									entity->SetSubEntityMaterial(material, subMeshIndex, lodIndex);
								}

								++lodIndex;
							}while(meshHead = meshHead->nextLod);
						}
					}
					
				}
				else if (nodeIter->lightName)
				{
					//create light
					std::map<MashStringc, sLightData, std::less<MashStringc>, lightAlloc >::iterator lightIter = lightMap.find(nodeIter->lightName);
					if (lightIter != lightMap.end())
					{
						MashLight *light = device->GetSceneManager()->AddLight(0, nameBuffer, lightIter->second.lightType, aLIGHT_RENDERER_FORWARD, false);
						nodeIter->engineNode = light;
					}
				}
				else if (nodeIter->cameraName)
				{
					//create camera
					MashCamera *camera = device->GetSceneManager()->AddCamera(0, nameBuffer);
					nodeIter->engineNode = camera;
				}
				else
				{
					//create dummy
					MashDummy *dummy = device->GetSceneManager()->AddDummy(0, nameBuffer);
					nodeIter->engineNode = dummy;
				}
			}

			if (nodeIter->engineNode)
			{
				mash::MashSceneNode *newSceneNode = nodeIter->engineNode;
				newSceneNode->SetTransformation(nodeIter->localTransform, true);

				//set up animations
				std::map<MashStringc, sAnimChannel, std::less<MashStringc>, animChannelAlloc >::iterator animChannelIter = animChannelMap.find(nodeIter->id);
				if (animChannelIter != animChannelMap.end())
				{
					MashAnimationBuffer *animBuffer = CreateAnimationBuffer(device, &animChannelIter->second, &(*nodeIter), loadSettings.frameRate);
					newSceneNode->SetAnimationBuffer(animBuffer);
					animBuffer->Drop();
				}
			}
		}

		//link up parents
		nodeIter = nodes.Begin();
		nodeIterEnd = nodes.End();
		for(; nodeIter != nodeIterEnd; ++nodeIter)
		{
			if (nodeIter->engineNode)
			{
				if ((nodeIter->parentNode) && nodeIter->parentNode->engineNode)
				{
					nodeIter->parentNode->engineNode->AddChild(nodeIter->engineNode);
				}
				else
				{
					if (rootNode)
					{
						rootNode->AddChild(nodeIter->engineNode);
					}
					else
						rootNodes.PushBack(nodeIter->engineNode);
				}
			}
		}

		std::map<MashStringc, sSkinController, std::less<MashStringc>, skinControllerAlloc >::iterator skinControllerIter = skinControllers.begin();
		std::map<MashStringc, sSkinController, std::less<MashStringc>, skinControllerAlloc >::iterator skinControllerIterEnd = skinControllers.end();
		for(; skinControllerIter != skinControllerIterEnd; ++skinControllerIter)
		{
			if (skinControllerIter->second.engineSkin)
			{
				sVariableArray *jointStringArray = 0;
				sArrayDataSource *bindPoseArray = 0;
				MashList<sInputData>::Iterator jointDataIter = skinControllerIter->second.jointData.Begin();
				MashList<sInputData>::Iterator jointDataIterEnd = skinControllerIter->second.jointData.End();
				for(; jointDataIter != jointDataIterEnd; ++jointDataIter)
				{
					if (jointDataIter->usage == aEXT_INPUT_SEMANTIC_JOINT)
						jointStringArray = &jointDataIter->sourceArray->arrayData.varaibleArray;
					else if (jointDataIter->usage == aEXT_INPUT_SEMANTIC_INV_BIND_MATRIX)
						bindPoseArray = &jointDataIter->sourceArray->arrayData;
				}
				
				if (!jointStringArray || !bindPoseArray)
					continue;

				uint32 currentBoneIndex = 0;
				mash::MashMatrix4 invBindPose;

				uint32 currentIndex = 0;
				uint32 stringStart = 0;

				int8 *nonConstJointStringArray = (int8*)jointStringArray->s;

				if (jointStringArray->s && (jointStringArray->s[0] != 0))
				{
					const uint32 bindPoseStride = math::Clamp<uint32>(0, 16, bindPoseArray->stride);

					while(true)
					{
						if ((jointStringArray->s[currentIndex] == 0) || isspace(jointStringArray->s[currentIndex]))
						{
							if (stringStart != currentIndex)//shouldnt happen anyway
							{
								int8 cBeforeNull = jointStringArray->s[currentIndex];
								nonConstJointStringArray[currentIndex] = 0;
								sNode *jointNode = 0;
								const int8 *jointName = &nonConstJointStringArray[stringStart];

								nodeIter = nodes.Begin();
								nodeIterEnd = nodes.End();
								for(; nodeIter != nodeIterEnd; ++nodeIter)
								{
									if (nodeIter->engineNode && (nodeIter->engineNode->GetNodeType() == aNODETYPE_BONE))
									{
										if (nodeIter->jointName && (strcmp(nodeIter->jointName, jointName) == 0))
										{
											jointNode = &(*nodeIter);
											break;
										}
									}
								}

								nonConstJointStringArray[currentIndex] = cBeforeNull;
								stringStart = currentIndex+1;
								//node may not be found. It cound be in another root
								if (jointNode)
								{
									memcpy(invBindPose.v, &bindPoseArray->varaibleArray.f[currentBoneIndex * bindPoseArray->stride], sizeof(f32) * bindPoseStride);
									ConvertMatrix(fileData.upAxis, invBindPose);
									MashBone *boneSceneNode = (MashBone*)jointNode->engineNode;
									boneSceneNode->SetWorldBindPose(invBindPose, true);

									boneSceneNode->SetLocalBindPose(boneSceneNode->GetLocalTransformState().translation, 
										boneSceneNode->GetLocalTransformState().orientation, 
										boneSceneNode->GetLocalTransformState().scale);

									skinControllerIter->second.engineSkin->AddBone(boneSceneNode, currentBoneIndex++);
								}
							}
						}

						if (jointStringArray->s[currentIndex] == 0)
							break;

						++currentIndex;
					}
				}
			}
		}

		std::map<MashStringc, sMesh*, std::less<MashStringc>, meshAlloc >::iterator modelMapIter = modelMap.begin();
		std::map<MashStringc, sMesh*, std::less<MashStringc>, meshAlloc >::iterator modelMapIterEnd = modelMap.end();
		for(; modelMapIter != modelMapIterEnd; ++modelMapIter)
		{
			if (modelMapIter->second)
			{
				if (modelMapIter->second->meshArray)
				{
					for(uint32 i = 0; i < modelMapIter->second->meshCount; ++i)
					{
						if (modelMapIter->second->meshArray[i].triangleIndices.f)
							m_memoryPool.FreeMemory(modelMapIter->second->meshArray[i].triangleIndices.i);
					}
					m_memoryPool.FreeMemory(modelMapIter->second->meshArray);
				}

				if (modelMapIter->second->engineModel)
					modelMapIter->second->engineModel->Drop();

				m_memoryPool.FreeMemory(modelMapIter->second);
			}
		}

		modelMap.clear();

		std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc >::iterator sourceMapIter = sourceMap.begin();
		std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc >::iterator sourceMapIterEnd = sourceMap.end();
		for(; sourceMapIter != sourceMapIterEnd; ++sourceMapIter)
		{
			//names arnt allocated into new memory
			if ((sourceMapIter->second.arrayData.varaibleArray.type != aARRAY_DATA_SOURCE_NAME) && sourceMapIter->second.arrayData.varaibleArray.f)
			{
				m_memoryPool.FreeMemory(sourceMapIter->second.arrayData.varaibleArray.f);
			}
		}

		sourceMap.clear();

		skinControllerIter = skinControllers.begin();
		skinControllerIterEnd = skinControllers.end();
		for(; skinControllerIter != skinControllerIterEnd; ++skinControllerIter)
		{
			if (skinControllerIter->second.engineSkin)
				skinControllerIter->second.engineSkin->Drop();

			if (skinControllerIter->second.vertexBoneCounts.f)
				m_memoryPool.FreeMemory(skinControllerIter->second.vertexBoneCounts.f););

			if (skinControllerIter->second.vertexBoneWeightIndices.f)
				m_memoryPool.FreeMemory(skinControllerIter->second.vertexBoneWeightIndices.f););
		}

		xmlReader->Destroy();

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
					"CMashColladaLoader::Load",
					"Collada load succeeded for file '%s'.",
					filename.GetCString());

		return aMASH_OK;

		//library_geometries
	}
}
	
