#include "stdafx.h"
#include "AnimationSetCore.h"

using std::shared_ptr;
using std::string;

std::unordered_map<std::string, std::shared_ptr<CAnimationSet>> CAnimationSets::s_GlobalAnimationSetCache;

CAnimationSet::CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrames, int nAnimatedBones, char* pstrName)
{
	m_fLength = fLength;
	m_nFramesPerSecond = nFramesPerSecond;
	m_nKeyFrames = nKeyFrames;

	strcpy_s(m_pstrAnimationSetName, 64, pstrName);

	m_pfKeyFrameTimes = new float[nKeyFrames];
	m_ppxmf4x4KeyFrameTransforms = new XMFLOAT4X4 * [nKeyFrames];
	for (int i = 0; i < nKeyFrames; i++) m_ppxmf4x4KeyFrameTransforms[i] = new XMFLOAT4X4[nAnimatedBones];
}

CAnimationSet::~CAnimationSet()
{
	if (m_pfKeyFrameTimes)
		delete[] m_pfKeyFrameTimes;

	for (int j = 0; j < m_nKeyFrames; j++)
		if (m_ppxmf4x4KeyFrameTransforms[j])
			delete[] m_ppxmf4x4KeyFrameTransforms[j];

	if (m_ppxmf4x4KeyFrameTransforms)
		delete[] m_ppxmf4x4KeyFrameTransforms;

	//DebugOutput("\nDelete AnimationSet: ", m_pstrAnimationSetName);
}

XMFLOAT4X4 CAnimationSet::GetSRT(int nBone, float fPosition)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Identity();

	if (fPosition >= m_pfKeyFrameTimes[m_nKeyFrames - 1])
		return m_ppxmf4x4KeyFrameTransforms[m_nKeyFrames - 1][nBone];

	for (int i = 0; i < (m_nKeyFrames - 1); i++)
	{
		if ((m_pfKeyFrameTimes[i] <= fPosition) && (fPosition < m_pfKeyFrameTimes[i + 1]))
		{
			float t = (fPosition - m_pfKeyFrameTimes[i]) / (m_pfKeyFrameTimes[i + 1] - m_pfKeyFrameTimes[i]);
			//xmf4x4Transform = Matrix4x4::Interpolate(m_ppxmf4x4KeyFrameTransforms[i][nBone], m_ppxmf4x4KeyFrameTransforms[i + 1][nBone], t);
			break;
		}
	}

	return(xmf4x4Transform);
}

std::unordered_map<std::string, shared_ptr<CAnimationSet>> CAnimationSets::s_globalCache;

CAnimationSets::CAnimationSets(int nAnimationSets)
{
	m_nAnimationSets = nAnimationSets;
	m_pAnimationSet_list.resize(nAnimationSets);
}

CAnimationSets::~CAnimationSets()
{
	m_pAnimationSet_list.clear();
}

std::shared_ptr<CAnimationSet> CAnimationSets::AddOrGetSharedAnimationSet(std::shared_ptr<CAnimationSet> animSet, const std::string& fileName)
{
	std::string key = fileName + "::" + animSet->m_pstrAnimationSetName;

	auto it = s_GlobalAnimationSetCache.find(key);
	if (it != s_GlobalAnimationSetCache.end())
		return it->second;

	s_GlobalAnimationSetCache[key] = animSet;
	return animSet;
}