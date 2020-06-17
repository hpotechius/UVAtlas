//-------------------------------------------------------------------------------------
// UVAtlas - basemeshinfo.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=512686
//-------------------------------------------------------------------------------------

#include "pch.h"
#include "isochartmesh.h"

using namespace DirectX;
using namespace Isochart;

namespace
{
    // Scale original model into the cubic with ISOCHART_MODELSCALE edge length. 500 is a experiential value
    const float ISOCHART_MODELSCALE = 500.0f;
}

CBaseMeshInfo::CBaseMeshInfo() :
    pVertexArray(nullptr),
    dwVertexCount(0),
    dwVertexStride(0),
    dwFaceCount(0),
    IndexFormat(DXGI_FORMAT_R16_UINT),
    pfIMTArray(nullptr),
    pdwOriginalFaceAdjacentArray(nullptr),
    pVertPosition(nullptr),
    pFaceNormalArray(nullptr),
    pFaceCanonicalUVCoordinate(nullptr),
    pFaceCanonicalParamAxis(nullptr),
    pfFaceAreaArray(nullptr),
    pdwFaceAdjacentArray(nullptr),
    fMeshArea(0),
    fBoxDiagLen(0),
    fOverturnTolerance(0),
    fExpectAvgL2SquaredStretch(0),
    fExpectMinAvgL2SquaredStretch(FACE_MIN_L2_STRETCH),
    fRatioOfSigToGeo(0),
    bIsFaceAdjacenctArrayReady(false),
    pdwSplitHint(nullptr)
{
}

CBaseMeshInfo::~CBaseMeshInfo()
{
    Free();
}

HRESULT CBaseMeshInfo::Initialize(
    const void* pfVertexArrayIn,
    size_t dwVertexCountIn,
    size_t dwVertexStrideIn,
    DXGI_FORMAT IndexFormatIn,
    const void* pdwFaceIndexArrayIn,
    size_t dwFaceCountIn,
    const FLOAT3* pfIMTArrayIn,
    const uint32_t* pdwFaceAdjacentArrayIn,
    const uint32_t* pdwSplitHintIn)
{
    HRESULT hr;

    assert(pfVertexArrayIn != nullptr);
    assert(pdwFaceIndexArrayIn != nullptr);
    assert(dwVertexStrideIn >= sizeof(float) * 3);
    assert((DXGI_FORMAT_R16_UINT == IndexFormatIn) || (DXGI_FORMAT_R32_UINT == IndexFormatIn));

    pVertexArray = pfVertexArrayIn;
    dwVertexCount = dwVertexCountIn;
    dwVertexStride = dwVertexStrideIn;
    pdwOriginalFaceAdjacentArray = pdwFaceAdjacentArrayIn;

    dwFaceCount = dwFaceCountIn;
    IndexFormat = IndexFormatIn;

    pfIMTArray = pfIMTArrayIn;
    pdwSplitHint = pdwSplitHintIn;

    if (FAILED(hr = CopyAndScaleInputVertices()))
    {
       goto LFail;
    }

    if (DXGI_FORMAT_R16_UINT == IndexFormat)
    {
        hr = ComputeInputFaceAttributes<uint16_t>(
            pdwFaceIndexArrayIn,
            pdwFaceAdjacentArrayIn);
    }

    else
    {
        hr = ComputeInputFaceAttributes<uint32_t>(
            pdwFaceIndexArrayIn,
            pdwFaceAdjacentArrayIn);
    }

    if (FAILED(hr))
    {
        goto LFail;
    }

    fOverturnTolerance = Overturn_TOLERANCE;
    return S_OK;
LFail:

    Free();
    return hr;
}

HRESULT CBaseMeshInfo::Initialize(
    const void* pfVertexArrayIn,
    size_t dwVertexCountIn,
    size_t dwVertexStrideIn,
    size_t dwFaceCountIn,
    const uint32_t* pdwFaceAdjacentArrayIn)
{
    assert(pfVertexArrayIn != nullptr);
    assert(dwVertexStrideIn >= sizeof(float) * 3);

    pVertexArray = pfVertexArrayIn;
    dwVertexCount = dwVertexCountIn;
    dwVertexStride = dwVertexStrideIn;
    dwFaceCount = dwFaceCountIn;
    pdwOriginalFaceAdjacentArray = pdwFaceAdjacentArrayIn;

    if (pdwFaceAdjacentArrayIn)
    {
        pdwFaceAdjacentArray = new (std::nothrow) uint32_t[3 * dwFaceCount];
        if (!pdwFaceAdjacentArray)
        {
            Free();
            return E_OUTOFMEMORY;
        }

        memcpy(
            pdwFaceAdjacentArray,
            pdwFaceAdjacentArrayIn,
            3 * dwFaceCount * sizeof(uint32_t));
        bIsFaceAdjacenctArrayReady = true;
    }

    return S_OK;
}

void CBaseMeshInfo::Free()
{
	/*SAFE_DELETE_ARRAY(pVertPosition)
        SAFE_DELETE_ARRAY(pFaceNormalArray)
        SAFE_DELETE_ARRAY(pfFaceAreaArray)
        SAFE_DELETE_ARRAY(pdwFaceAdjacentArray)
        SAFE_DELETE_ARRAY(pFaceCanonicalUVCoordinate)
        SAFE_DELETE_ARRAY(pFaceCanonicalParamAxis)*/

        pfIMTArray = nullptr;

    dwVertexCount = 0;
    dwFaceCount = 0;
    fBoxDiagLen = 0;
    fMeshArea = 0;

    pdwSplitHint = nullptr;
}

HRESULT CBaseMeshInfo::CopyAndScaleInputVertices()
{
    pVertPosition = new (std::nothrow) glm::vec3[dwVertexCount];
    //pVertPosition = new (std::nothrow) XMFLOAT3[dwVertexCount];
    if (!pVertPosition)
    {
    	printError(E_OUTOFMEMORY, __FILE__, __FUNCTION__, __LINE__);
        return E_OUTOFMEMORY;
    }

    glm::vec3 vMinCoords = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 vMaxCoords = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    /*XMFLOAT3 vMinCoords(
        FLT_MAX,
        FLT_MAX,
        FLT_MAX);

    XMFLOAT3 vMaxCoords(
        -FLT_MAX,
        -FLT_MAX,
        -FLT_MAX);*/


    float* pfMaxVector = &vMaxCoords.x;
    float* pfMinVector = &vMinCoords.x;

    auto pVertexBuffer = static_cast<uint8_t*>(const_cast<void*>(pVertexArray));
    float* pVertexCoord = nullptr;

    for (size_t i = 0; i < dwVertexCount; i++)
    {
        pVertexCoord = reinterpret_cast<float*>(pVertexBuffer);
        for (size_t j = 0; j < 3; j++)
        {
            if (pfMinVector[j] > pVertexCoord[j])
            {
                pfMinVector[j] = pVertexCoord[j];
            }
            if (pfMaxVector[j] < pVertexCoord[j])
            {
                pfMaxVector[j] = pVertexCoord[j];
            }

        }
        pVertexBuffer += dwVertexStride;
    }

    /*XMFLOAT3*/ glm::vec3 vCenter;
    vCenter.x = (vMinCoords.x + vMaxCoords.x) / 2.0f;
    vCenter.y = (vMinCoords.y + vMaxCoords.y) / 2.0f;
    vCenter.z = (vMinCoords.z + vMaxCoords.z) / 2.0f;
    glm::vec4 vvCenter = glm::vec4(vCenter, 0.0f);
    //XMVECTOR vvCenter = XMLoadFloat3(&vCenter);

    float scale = 1.0f;


    scale = std::max(vMaxCoords.x - vMinCoords.x, std::max(vMaxCoords.y - vMinCoords.y, vMaxCoords.z - vMinCoords.z));

    // either all vertices are the same or there are NaN's involved, just keep
    // the same scale in this case
    if (scale <= 0.0f)
        scale = 1.0f;

    scale = ISOCHART_MODELSCALE / scale;

    //DPF(0, "Scale factor is %f", double(scale));
    std::string message = "Scale factor is " + std::to_string(double(scale));
    printProgress(__FILE__, __FUNCTION__, __LINE__, message);
    pVertexBuffer = static_cast<uint8_t*>(const_cast<void*>(pVertexArray));
    pVertexCoord = nullptr;

    for (size_t i = 0; i < dwVertexCount; i++)
    {
        pVertexCoord = static_cast<float*>(static_cast<void*>(pVertexBuffer));
        glm::vec4 vVertPos = glm::vec4(pVertexCoord[0], pVertexCoord[1], pVertexCoord[2], 0.0f);
        //XMVECTOR vVertPos = XMVectorSet(pVertexCoord[0], pVertexCoord[1], pVertexCoord[2], 0);
        vVertPos -= vvCenter;
        vVertPos *= scale;
        pVertPosition[i] = glm::vec3(vVertPos.x,vVertPos.y,vVertPos.z);
        //XMStoreFloat3(&pVertPosition[i], vVertPos);

        pVertexBuffer += dwVertexStride;
    }

    glm::vec4 vvMaxCoords = glm::vec4(vMaxCoords, 0.0f);
    glm::vec4 vvMinCoords = glm::vec4(vMinCoords, 0.0f);
    //XMVECTOR vvMaxCoords = XMLoadFloat3(&vMaxCoords);
    //XMVECTOR vvMinCoords = XMLoadFloat3(&vMinCoords);

    vvMaxCoords -= vvCenter;
    vvMaxCoords *= scale;
    vvMinCoords -= vvCenter;
    vvMinCoords *= scale;

    glm::vec4 tempVV = vvMaxCoords - vvMinCoords;
    fBoxDiagLen = glm::length(glm::vec3(tempVV.x, tempVV.y, tempVV.z));
    //fBoxDiagLen = XMVectorGetX(XMVector3Length(vvMaxCoords - vvMinCoords));

    return S_OK;
}

template <class INDEXTYPE>
HRESULT CBaseMeshInfo::ComputeInputFaceAttributes(
    const void* pdwFaceIndexArrayIn,
    const uint32_t* pdwFaceAdjacentArrayIn)
{
    assert(pdwFaceIndexArrayIn != nullptr);

    pFaceNormalArray = new (std::nothrow) glm::vec3[dwFaceCount];
    //pFaceNormalArray = new (std::nothrow) XMFLOAT3[dwFaceCount];
    if (!pFaceNormalArray)
    {
        Free();
        printError(E_OUTOFMEMORY, __FILE__, __FUNCTION__, __LINE__);
        return E_OUTOFMEMORY;
    }

    pfFaceAreaArray = new (std::nothrow) float[dwFaceCount];
    if (!pfFaceAreaArray)
    {
        Free();
        printError(E_OUTOFMEMORY, __FILE__, __FUNCTION__, __LINE__);
        return E_OUTOFMEMORY;
    }

    pdwFaceAdjacentArray = new (std::nothrow) uint32_t[3 * dwFaceCount];
    if (!pdwFaceAdjacentArray)
    {
        Free();
        printError(E_OUTOFMEMORY, __FILE__, __FUNCTION__, __LINE__);
        return E_OUTOFMEMORY;
    }

    // Need to use face canonical coordinates.
    if (pfIMTArray)
    {
    	pFaceCanonicalUVCoordinate = new (std::nothrow) glm::vec2[3 * dwFaceCount];
        //pFaceCanonicalUVCoordinate = new (std::nothrow) XMFLOAT2[3 * dwFaceCount];
        if (!pFaceCanonicalUVCoordinate)
        {
            Free();
            printError(E_OUTOFMEMORY, __FILE__, __FUNCTION__, __LINE__);
            return E_OUTOFMEMORY;
        }

    	pFaceCanonicalParamAxis = new (std::nothrow) glm::vec3[2 * dwFaceCount];
        //pFaceCanonicalParamAxis = new (std::nothrow) XMFLOAT3[2 * dwFaceCount];
        if (!pFaceCanonicalParamAxis)
        {
            Free();
            printError(E_OUTOFMEMORY, __FILE__, __FUNCTION__, __LINE__);
            return E_OUTOFMEMORY;
        }
    }

    /*XMFLOAT2**/ glm::vec2* pCoordinate = pFaceCanonicalUVCoordinate;
    /*XMFLOAT3**/ glm::vec3* pAxis = pFaceCanonicalParamAxis;

    // Compute the normal and area of each face
    const INDEXTYPE* pFace = static_cast<INDEXTYPE*>(const_cast<void*>(pdwFaceIndexArrayIn));
    fMeshArea = 0;

    //std::cout << "pFace - " <<  << std::endl;
    for (size_t i = 0; i < dwFaceCount; i++)
    {
//    	std::cout << i << " pFace0 - " << pFace[0] << std::endl;
//    	std::cout << i << " pFace1 - " << pFace[1] << std::endl;
//    	std::cout << i << " pFace2 - " << pFace[2] << std::endl;
        glm::vec4 v0 = glm::vec4(pVertPosition[pFace[1]], 0.0f) - glm::vec4(pVertPosition[pFace[0]], 0.0f);
        glm::vec4 v1 = glm::vec4(pVertPosition[pFace[2]], 0.0f) - glm::vec4(pVertPosition[pFace[0]], 0.0f);
        glm::vec3* pFaceNormal = pFaceNormalArray + i;
        //XMVECTOR v0 = XMLoadFloat3(&pVertPosition[pFace[1]]) - XMLoadFloat3(&pVertPosition[pFace[0]]);
        //XMVECTOR v1 = XMLoadFloat3(&pVertPosition[pFace[2]]) - XMLoadFloat3(&pVertPosition[pFace[0]]);
        //XMFLOAT3* pFaceNormal = pFaceNormalArray + i;

        glm::vec4 vFaceNormal = glm::vec4(glm::cross(glm::vec3(v0.x,v0.y,v0.z),glm::vec3(v1.x,v1.y,v1.z)),0.0f);
        *pFaceNormal = glm::vec3(vFaceNormal.x,vFaceNormal.y,vFaceNormal.z);
        float area = glm::length(*pFaceNormal);
        //XMVECTOR vFaceNormal = XMVector3Cross(v0, v1);
        //XMStoreFloat3(pFaceNormal, vFaceNormal);
        //float area = XMVectorGetX(XMVector3Length(vFaceNormal));

        pfFaceAreaArray[i] = area * 0.5f;
        fMeshArea += pfFaceAreaArray[i];
        if (area > 0.f)
            vFaceNormal /= area;

        *pFaceNormal = glm::vec3(vFaceNormal.x,vFaceNormal.y,vFaceNormal.z);
        //XMStoreFloat3(pFaceNormal, vFaceNormal);

        if (pFaceCanonicalUVCoordinate)
        {
            CaculateCanonicalCoordinates(
                pVertPosition + pFace[0],
                pVertPosition + pFace[1],
                pVertPosition + pFace[2],
                pCoordinate,
                pCoordinate + 1,
                pCoordinate + 2,
                pAxis);

            pCoordinate += 3;
            pAxis += 2;
        }
        pFace += 3;
    }

    if (pdwFaceAdjacentArrayIn)
    {
        memcpy(pdwFaceAdjacentArray, pdwFaceAdjacentArrayIn, 3 * dwFaceCount * sizeof(uint32_t));
        bIsFaceAdjacenctArrayReady = true;
    }
    return S_OK;
}

void CBaseMeshInfo::CaculateCanonicalCoordinates(
    const /*XMFLOAT3**/ glm::vec3* pv3D0,
    const /*XMFLOAT3**/ glm::vec3* pv3D1,
    const /*XMFLOAT3**/ glm::vec3* pv3D2,
    /*XMFLOAT2**/ glm::vec2* pv2D0,
    /*XMFLOAT2**/ glm::vec2* pv2D1,
    /*XMFLOAT2**/ glm::vec2* pv2D2,
    /*XMFLOAT3**/ glm::vec3* pAxis)
{
    IsochartCaculateCanonicalCoordinates(
        pv3D0,
        pv3D1,
        pv3D2,
        pv2D0,
        pv2D1,
        pv2D2,
        pAxis);
}
