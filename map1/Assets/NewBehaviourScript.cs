using UnityEngine;
using System.Collections.Generic;

public class PirateVillageGenerator : MonoBehaviour
{
    [Header("Prefabs")]
    public GameObject[] housePrefabs;
    public GameObject[] shopPrefabs;
    public GameObject[] treePrefabs;
    public GameObject[] marketPrefabs; // 시장/천막 등
    public GameObject plazaPrefab;     // 분수대/우물/광장 소품

    [Header("Props Near House")]
    public GameObject[] barrelPrefabs;
    public GameObject[] cratePrefabs;
    public GameObject[] fencePrefabs;
    public GameObject[] decorPrefabs; // 표지판, 랜턴 등

    [Header("Settings")]
    public int houseCount = 26;
    public int shopCount = 16;
    public int treeCount = 55;
    public int marketCount = 15;
    public float plazaRadius = 35f;
    public float villageRadius = 140f;

    public int propPerHouseMin = 2;  // 건물당 소품 최소
    public int propPerHouseMax = 5;  // 건물당 소품 최대
    public float propMinDist = 2.2f; // 건물과 소품 거리(최소)
    public float propMaxDist = 4.2f; // 건물과 소품 거리(최대)
    public float minDistBetweenObjects = 2.2f; // 오브젝트간 최소거리

    List<Vector3> allPositions = new List<Vector3>(); // 전체 배치 좌표 기억 (겹침 방지)

    void Start()
    {
        Vector3 plazaCenter = Vector3.zero;

        // 1. 중앙 광장: 분수대(우물 등)
        if (plazaPrefab != null)
        {
            PlaceObject(plazaPrefab, plazaCenter);
        }

        // 2. 시장/천막: 광장 반경 안쪽에만 배치
        for (int i = 0; i < marketCount; i++)
        {
            Vector3 pos = RandomCircle(plazaCenter, Random.Range(plazaRadius * 0.65f, plazaRadius * 0.95f));
            if (TryPlace(pos, minDistBetweenObjects))
                PlaceObject(marketPrefabs, pos);
        }

        // 3. 집/상점: 광장 주변~외곽에 배치
        List<Vector3> housePositions = new List<Vector3>();
        for (int i = 0; i < houseCount; i++)
        {
            Vector3 pos = RandomCircle(plazaCenter, Random.Range(plazaRadius + 9f, villageRadius - 13f));
            if (TryPlace(pos, minDistBetweenObjects * 2))
            {
                PlaceObject(housePrefabs, pos, true);
                housePositions.Add(pos);
            }
        }
        for (int i = 0; i < shopCount; i++)
        {
            Vector3 pos = RandomCircle(plazaCenter, Random.Range(plazaRadius + 7f, villageRadius - 10f));
            if (TryPlace(pos, minDistBetweenObjects * 2))
            {
                PlaceObject(shopPrefabs, pos, true);
                housePositions.Add(pos);
            }
        }

        // 4. 집/상점 주변: 소품 군집 랜덤 배치 (술통, 상자, 울타리 등)
        foreach (Vector3 basePos in housePositions)
        {
            int propNum = Random.Range(propPerHouseMin, propPerHouseMax + 1);
            for (int n = 0; n < propNum; n++)
            {
                GameObject[] propArr = PickPropArray();
                if (propArr == null || propArr.Length == 0) continue;

                float angle = Random.Range(0, Mathf.PI * 2);
                float dist = Random.Range(propMinDist, propMaxDist);
                Vector3 pos = basePos + new Vector3(Mathf.Cos(angle), 0, Mathf.Sin(angle)) * dist;

                if (TryPlace(pos, minDistBetweenObjects * 0.7f))
                    PlaceObject(propArr, pos);
            }
        }

        // 5. 외곽/빈 공간: 나무, 장식 등
        for (int i = 0; i < treeCount; i++)
        {
            Vector3 pos = RandomCircle(plazaCenter, Random.Range(plazaRadius + 11f, villageRadius));
            if (TryPlace(pos, minDistBetweenObjects * 1.2f))
                PlaceObject(treePrefabs, pos);
        }
        for (int i = 0; i < Mathf.Max(15, houseCount / 2); i++)
        {
            Vector3 pos = RandomCircle(plazaCenter, Random.Range(plazaRadius + 7f, villageRadius));
            if (TryPlace(pos, minDistBetweenObjects))
                PlaceObject(decorPrefabs, pos);
        }
    }

    // --- 유틸리티 함수들 ---
    Vector3 RandomCircle(Vector3 center, float radius)
    {
        float angle = Random.Range(0, Mathf.PI * 2);
        return center + new Vector3(Mathf.Cos(angle), 0, Mathf.Sin(angle)) * radius;
    }

    GameObject[] PickPropArray()
    {
        int roll = Random.Range(0, 4);
        if (roll == 0 && barrelPrefabs.Length > 0) return barrelPrefabs;
        if (roll == 1 && cratePrefabs.Length > 0) return cratePrefabs;
        if (roll == 2 && fencePrefabs.Length > 0) return fencePrefabs;
        if (roll == 3 && decorPrefabs.Length > 0) return decorPrefabs;
        return null;
    }

    void PlaceObject(GameObject[] prefabs, Vector3 pos, bool randomY = false)
    {
        if (prefabs == null || prefabs.Length == 0) return;
        GameObject pf = prefabs[Random.Range(0, prefabs.Length)];
        PlaceObject(pf, pos, randomY);
    }
    void PlaceObject(GameObject pf, Vector3 pos, bool randomY = false)
    {
        if (!pf) return;
        Quaternion rot = Quaternion.Euler(0, Random.Range(0, 360), 0);
        if (randomY)
            rot = Quaternion.Euler(0, Random.Range(0, 360), Random.Range(-5, 5)); // Y뿐만 아니라 약간 틀어지게
        Instantiate(pf, pos, rot, transform);
        allPositions.Add(pos);
    }

    bool TryPlace(Vector3 pos, float minDist)
    {
        foreach (var v in allPositions)
            if ((v - pos).sqrMagnitude < minDist * minDist)
                return false;
        return true;
    }
}
