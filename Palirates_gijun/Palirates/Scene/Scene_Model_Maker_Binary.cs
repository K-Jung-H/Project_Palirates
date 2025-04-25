using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using UnityEditor;
using System.Text;

public class Scene_Model_Maker_Binary : MonoBehaviour
{
    public string sceneFileName = "Scene_Name";


    private List<string> m_pTextureNamesListForCounting = new List<string>();
    private List<string> m_pTextureNamesListForWriting = new List<string>();

    private BinaryWriter global_binaryWriter = null;
    private BinaryWriter temp_BinaryWriter = null;

    private int m_nFrames = 0;

    bool FindTextureByName(List<string> pTextureNamesList, Texture texture)
    {
        if (texture)
        {
            string strTextureName = string.Copy(texture.name).Replace(" ", "_");
            for (int i = 0; i < pTextureNamesList.Count; i++)
            {
                if (pTextureNamesList.Contains(strTextureName)) return (true);
            }
            pTextureNamesList.Add(strTextureName);
            return (false);
        }
        else
        {
            return (true);
        }
    }

    void WriteObjectName(Object obj)
    {
        global_binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteObjectName(int i, Object obj)
    {
        global_binaryWriter.Write(i);
        global_binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteObjectName(string strHeader, Object obj)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteObjectName(string strHeader, int i, Object obj)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(i);
        global_binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteObjectName(string strHeader, int i, int j, Object obj)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(i);
        global_binaryWriter.Write(j);
        global_binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteObjectName(string strHeader, int i, Object obj, float f, int j, int k)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(i);
        global_binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
        global_binaryWriter.Write(f);
        global_binaryWriter.Write(j);
        global_binaryWriter.Write(k);
    }

    void WriteString(string strToWrite)
    {
        global_binaryWriter.Write(strToWrite);
    }

    void WriteString(string strHeader, string strToWrite)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(strToWrite);
    }

    void WriteString(string strToWrite, int i)
    {
        global_binaryWriter.Write(strToWrite);
        global_binaryWriter.Write(i);
    }

    void WriteString(string strToWrite, int i, float f)
    {
        global_binaryWriter.Write(strToWrite);
        global_binaryWriter.Write(i);
        global_binaryWriter.Write(f);
    }

    void WriteTextureName(string strHeader, Texture texture)
    {
        global_binaryWriter.Write(strHeader);
        if (texture)
        {
            if (FindTextureByName(m_pTextureNamesListForWriting, texture))
            {
                global_binaryWriter.Write("@" + string.Copy(texture.name).Replace(" ", "_"));
            }
            else
            {
                global_binaryWriter.Write(string.Copy(texture.name).Replace(" ", "_"));
            }
        }
        else
        {
            global_binaryWriter.Write("null");
        }
    }

    void WriteInteger(int i)
    {
        global_binaryWriter.Write(i);
    }

    void WriteInteger(string strHeader, int i)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(i);
    }

    void WriteFloat(string strHeader, float f)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(f);
    }

    void WriteVector(Vector2 v)
    {
        global_binaryWriter.Write(v.x);
        global_binaryWriter.Write(v.y);
    }

    void WriteVector(string strHeader, Vector2 v)
    {
        global_binaryWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Vector3 v)
    {
        global_binaryWriter.Write(v.x);
        global_binaryWriter.Write(v.y);
        global_binaryWriter.Write(v.z);
    }

    void WriteVector(string strHeader, Vector3 v)
    {
        global_binaryWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Vector4 v)
    {
        global_binaryWriter.Write(v.x);
        global_binaryWriter.Write(v.y);
        global_binaryWriter.Write(v.z);
        global_binaryWriter.Write(v.w);
    }

    void WriteVector(string strHeader, Vector4 v)
    {
        global_binaryWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Quaternion q)
    {
        global_binaryWriter.Write(q.x);
        global_binaryWriter.Write(q.y);
        global_binaryWriter.Write(q.z);
        global_binaryWriter.Write(q.w);
    }

    void WriteVector(string strHeader, Quaternion q)
    {
        global_binaryWriter.Write(strHeader);
        WriteVector(q);
    }

    void WriteColor(Color c)
    {
        global_binaryWriter.Write(c.r);
        global_binaryWriter.Write(c.g);
        global_binaryWriter.Write(c.b);
        global_binaryWriter.Write(c.a);
    }

    void WriteColor(string strHeader, Color c)
    {
        global_binaryWriter.Write(strHeader);
        WriteColor(c);
    }

    void WriteTextureCoord(Vector2 uv)
    {
        global_binaryWriter.Write(uv.x);
        global_binaryWriter.Write(1.0f - uv.y);
    }

    void WriteVectors(string strHeader, Vector2[] vectors)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(vectors.Length);
        if (vectors.Length > 0) foreach (Vector2 v in vectors) WriteVector(v);
    }

    void WriteVectors(string strHeader, Vector3[] vectors)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(vectors.Length);
        if (vectors.Length > 0) foreach (Vector3 v in vectors) WriteVector(v);
    }

    void WriteVectors(string strHeader, Vector4[] vectors)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(vectors.Length);
        if (vectors.Length > 0) foreach (Vector4 v in vectors) WriteVector(v);
    }

    void WriteColors(string strHeader, Color[] colors)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(colors.Length);
        if (colors.Length > 0) foreach (Color c in colors) WriteColor(c);
    }

    void WriteTextureCoords(string strHeader, Vector2[] uvs)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(uvs.Length);
        if (uvs.Length > 0) foreach (Vector2 uv in uvs) WriteTextureCoord(uv);
    }

    void WriteIntegers(int[] pIntegers)
    {
        global_binaryWriter.Write(pIntegers.Length);
        foreach (int i in pIntegers) global_binaryWriter.Write(i);
    }

    void WriteIntegers(string strHeader, int[] pIntegers)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(pIntegers.Length);
        if (pIntegers.Length > 0) foreach (int i in pIntegers) global_binaryWriter.Write(i);
    }

    void WriteIntegers(string strHeader, int n, int[] pIntegers)
    {
        global_binaryWriter.Write(strHeader);
        global_binaryWriter.Write(n);
        global_binaryWriter.Write(pIntegers.Length);
        if (pIntegers.Length > 0) foreach (int i in pIntegers) global_binaryWriter.Write(i);
    }

    void WriteBoundingBox(string strHeader, Bounds bounds)
    {
        global_binaryWriter.Write(strHeader);
        WriteVector(bounds.center);
        WriteVector(bounds.extents);
    }

    void WriteMatrix(Matrix4x4 matrix)
    {
        global_binaryWriter.Write(matrix.m00);
        global_binaryWriter.Write(matrix.m10);
        global_binaryWriter.Write(matrix.m20);
        global_binaryWriter.Write(matrix.m30);
        global_binaryWriter.Write(matrix.m01);
        global_binaryWriter.Write(matrix.m11);
        global_binaryWriter.Write(matrix.m21);
        global_binaryWriter.Write(matrix.m31);
        global_binaryWriter.Write(matrix.m02);
        global_binaryWriter.Write(matrix.m12);
        global_binaryWriter.Write(matrix.m22);
        global_binaryWriter.Write(matrix.m32);
        global_binaryWriter.Write(matrix.m03);
        global_binaryWriter.Write(matrix.m13);
        global_binaryWriter.Write(matrix.m23);
        global_binaryWriter.Write(matrix.m33);
    }

    void WriteMatrix(Vector3 position, Quaternion rotation, Vector3 scale)
    {
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(position, rotation, scale);
        WriteMatrix(matrix);
    }

    void WriteTransform(string strHeader, Transform current)
    {
        global_binaryWriter.Write(strHeader);
        WriteVector(current.localPosition);
        WriteVector(current.localEulerAngles);
        WriteVector(current.localScale);
        WriteVector(current.localRotation);
    }

    void WriteLocalMatrix(string strHeader, Transform current)
    {
        global_binaryWriter.Write(strHeader);
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(current.localPosition, current.localRotation, current.localScale);
        WriteMatrix(matrix);
    }

    void WriteWorldMatrix(string strHeader, Transform current)
    {
        global_binaryWriter.Write(strHeader);
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(current.position, current.rotation, current.lossyScale);
        WriteMatrix(matrix);
    }


    void WriteMatrixes(string strHeader, Matrix4x4[] matrixes)
    {
        WriteString(strHeader, matrixes.Length);
        if (matrixes.Length > 0)
        {
            foreach (Matrix4x4 matrix in matrixes) WriteMatrix(matrix);
        }
    }

//================================================================================





    int GetTexturesCount(Material[] materials)
    {
        int nTextures = 0;
        for (int i = 0; i < materials.Length; i++)
        {
            if (materials[i].HasProperty("_MainTex"))
            {
                if (!FindTextureByName(m_pTextureNamesListForCounting, materials[i].GetTexture("_MainTex"))) nTextures++;
            }
            if (materials[i].HasProperty("_SpecGlossMap"))
            {
                if (!FindTextureByName(m_pTextureNamesListForCounting, materials[i].GetTexture("_SpecGlossMap"))) nTextures++;
            }
            if (materials[i].HasProperty("_MetallicGlossMap"))
            {
                if (!FindTextureByName(m_pTextureNamesListForCounting, materials[i].GetTexture("_MetallicGlossMap"))) nTextures++;
            }
            if (materials[i].HasProperty("_BumpMap"))
            {
                if (!FindTextureByName(m_pTextureNamesListForCounting, materials[i].GetTexture("_BumpMap"))) nTextures++;
            }
            if (materials[i].HasProperty("_EmissionMap"))
            {
                if (!FindTextureByName(m_pTextureNamesListForCounting, materials[i].GetTexture("_EmissionMap"))) nTextures++;
            }
            if (materials[i].HasProperty("_DetailAlbedoMap"))
            {
                if (!FindTextureByName(m_pTextureNamesListForCounting, materials[i].GetTexture("_DetailAlbedoMap"))) nTextures++;
            }
            if (materials[i].HasProperty("_DetailNormalMap"))
            {
                if (!FindTextureByName(m_pTextureNamesListForCounting, materials[i].GetTexture("_DetailNormalMap"))) nTextures++;
            }
        }
        return (nTextures);
    }

    int GetTexturesCount(Transform current)
    {
        int nTextures = 0;
        MeshRenderer meshRenderer = current.gameObject.GetComponent<MeshRenderer>();
        if (meshRenderer) nTextures = GetTexturesCount(meshRenderer.materials);

        for (int k = 0; k < current.childCount; k++) nTextures += GetTexturesCount(current.GetChild(k));

        return (nTextures);
    }



    void WriteMeshInfo(Mesh mesh)
    {
        string meshFileName = mesh.name.Replace(" ", "_");

        string baseDirectoryPath = Path.Combine(Application.dataPath, "Scene_File", "Meshes");

        // bin 폴더와 txt 폴더 생성
        string binDirectoryPath = Path.Combine(baseDirectoryPath, "bin");

        if (!Directory.Exists(binDirectoryPath))
        {
            Directory.CreateDirectory(binDirectoryPath);
            Debug.Log("bin 폴더 생성됨.");
        }


        // bin 형식 파일 경로
        string binFilePath = Path.Combine(binDirectoryPath, meshFileName + ".bin");


        // 이미 저장된 메시 파일이 존재하면 생략
        if (File.Exists(binFilePath))
        {
            Debug.Log("Mesh already saved: " + meshFileName);
            return;  // 파일이 이미 존재하면 저장하지 않음
        }

        temp_BinaryWriter = global_binaryWriter;

        // bin 형식으로 저장
        using (BinaryWriter meshWriter = new BinaryWriter(File.Open(binFilePath, FileMode.Create)))
        {
            global_binaryWriter = meshWriter; 

            WriteObjectName("<Mesh>:", mesh.vertexCount, mesh);
            WriteBoundingBox("<Bounds>:", mesh.bounds);
            WriteVectors("<Positions>:", mesh.vertices);
            WriteColors("<Colors>:", mesh.colors);
            WriteTextureCoords("<TextureCoords0>:", mesh.uv);
            WriteTextureCoords("<TextureCoords1>:", mesh.uv2);
            WriteVectors("<Normals>:", mesh.normals);
        

            if ((mesh.normals.Length > 0) && (mesh.tangents.Length > 0))
            {
                Vector3[] tangents = new Vector3[mesh.tangents.Length];
                Vector3[] biTangents = new Vector3[mesh.tangents.Length];
                for (int i = 0; i < mesh.tangents.Length; i++)
                {
                    tangents[i] = new Vector3(mesh.tangents[i].x, mesh.tangents[i].y, mesh.tangents[i].z);
                    biTangents[i] = Vector3.Normalize(Vector3.Cross(mesh.normals[i], tangents[i])) * mesh.tangents[i].w;
                }

                WriteVectors("<Tangents>:", tangents);
                WriteVectors("<BiTangents>:", biTangents);
            }

            WriteInteger("<SubMeshes>:", mesh.subMeshCount);
            for (int i = 0; i < mesh.subMeshCount; i++)
            {
                int[] subIndices = mesh.GetTriangles(i);
                WriteIntegers("<SubMesh>:", i, subIndices);
            }

            meshWriter.Write("</Mesh>");

            global_binaryWriter = temp_BinaryWriter;
        }

        Debug.Log("Mesh info saved to bin files for: " + meshFileName);
    }


    void WriteMaterials(Material[] materials)
    {
        WriteInteger("<Materials>:", materials.Length);
        for (int i = 0; i < materials.Length; i++)
        {
            WriteInteger("<Material>:", i);

            if (materials[i].HasProperty("_Color"))
            {
                Color albedo = materials[i].GetColor("_Color");
                WriteColor("<AlbedoColor>:", albedo);
            }
            if (materials[i].HasProperty("_EmissionColor"))
            {
                Color emission = materials[i].GetColor("_EmissionColor");
                WriteColor("<EmissiveColor>:", emission);
            }
            if (materials[i].HasProperty("_SpecColor"))
            {
                Color specular = materials[i].GetColor("_SpecColor");
                WriteColor("<SpecularColor>:", specular);
            }
            if (materials[i].HasProperty("_Glossiness"))
            {
                WriteFloat("<Glossiness>:", materials[i].GetFloat("_Glossiness"));
            }
            if (materials[i].HasProperty("_Smoothness"))
            {
                WriteFloat("<Smoothness>:", materials[i].GetFloat("_Smoothness"));
            }
            if (materials[i].HasProperty("_Metallic"))
            {
                WriteFloat("<Metallic>:", materials[i].GetFloat("_Metallic"));
            }
            if (materials[i].HasProperty("_SpecularHighlights"))
            {
                WriteFloat("<SpecularHighlight>:", materials[i].GetFloat("_SpecularHighlights"));
            }
            if (materials[i].HasProperty("_GlossyReflections"))
            {
                WriteFloat("<GlossyReflection>:", materials[i].GetFloat("_GlossyReflections"));
            }

            if (materials[i].HasProperty("_MainTex"))
            {
                Texture mainAlbedoMap = materials[i].GetTexture("_MainTex");
                WriteTextureName("<AlbedoMap>:", mainAlbedoMap);
            }
            if (materials[i].HasProperty("_SpecGlossMap"))
            {
                Texture specularcMap = materials[i].GetTexture("_SpecGlossMap");
                WriteTextureName("<SpecularMap>:", specularcMap);
            }
            if (materials[i].HasProperty("_MetallicGlossMap"))
            {
                Texture metallicMap = materials[i].GetTexture("_MetallicGlossMap");
                WriteTextureName("<MetallicMap>:", metallicMap);
            }
            if (materials[i].HasProperty("_BumpMap"))
            {
                Texture bumpMap = materials[i].GetTexture("_BumpMap");
                WriteTextureName("<NormalMap>:", bumpMap);
            }
            if (materials[i].HasProperty("_EmissionMap"))
            {
                Texture emissionMap = materials[i].GetTexture("_EmissionMap");
                WriteTextureName("<EmissionMap>:", emissionMap);
            }
            if (materials[i].HasProperty("_DetailAlbedoMap"))
            {
                Texture detailAlbedoMap = materials[i].GetTexture("_DetailAlbedoMap");
                WriteTextureName("<DetailAlbedoMap>:", detailAlbedoMap);
            }
            if (materials[i].HasProperty("_DetailNormalMap"))
            {
                Texture detailNormalMap = materials[i].GetTexture("_DetailNormalMap");
                WriteTextureName("<DetailNormalMap>:", detailNormalMap);
            }
        }
        WriteString("</Materials>");
    }


    void WriteFrameInfo(Transform current)
    {
        int nTextures = GetTexturesCount(current);
        WriteObjectName("<Frame>:", m_nFrames++, nTextures, current.gameObject);

        WriteTransform("<Transform>:", current);
        WriteLocalMatrix("<TransformMatrix>:", current);

        MeshFilter meshFilter = current.gameObject.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = current.gameObject.GetComponent<MeshRenderer>();

        if (meshFilter && meshRenderer)
        {
            string meshFileName = meshFilter.sharedMesh.name.Replace(" ", "_") + ".bin";

            WriteString("<Mesh>:");
            WriteString("<Mesh_Name>:", meshFileName);
            WriteString("</Mesh>");

            WriteMeshInfo(meshFilter.sharedMesh);

            Material[] materials = meshRenderer.materials;
            if (materials.Length > 0) WriteMaterials(materials);
        }
    }

    void WriteFrameHierarchyInfo(Transform child)
    {
        WriteFrameInfo(child);

        WriteInteger("<Children>:", child.childCount);

        if (child.childCount > 0)
        {
            for (int k = 0; k < child.childCount; k++)
            {
                WriteFrameHierarchyInfo(child.GetChild(k));
            }
        }

        WriteString("</Frame>");
    }


    void WriteFrameHierarchyInfo(Transform child, HashSet<Transform> visitedTransforms)
    {
        if (visitedTransforms.Contains(child))
            return;

        visitedTransforms.Add(child);

        WriteFrameInfo(child);

        WriteInteger("<Children>:", child.childCount);

        if (child.childCount > 0)
        {
            for (int k = 0; k < child.childCount; k++)
            {
                WriteFrameHierarchyInfo(child.GetChild(k), visitedTransforms);
            }
        }

        WriteString("</Frame>");
    }

    void Start()
    {
        string sceneFileDirectory = Path.Combine(Application.dataPath, "Scene_File");

        if (!Directory.Exists(sceneFileDirectory))
        {
            Directory.CreateDirectory(sceneFileDirectory);
            Debug.Log("Scene_File 폴더 생성됨.");
        }

        string meshesDirectory = Path.Combine(sceneFileDirectory, "Meshes");

        if (!Directory.Exists(meshesDirectory))
        {
            Directory.CreateDirectory(meshesDirectory);
            Debug.Log("Meshes 폴더 생성됨.");
        }

        // 경로에 확장자를 추가하여 파일 경로를 제대로 설정
        string sceneFilePath = Path.Combine(sceneFileDirectory, sceneFileName + ".bin");

        // binaryWriter를 생성할 때 경로를 정확히 지정
        using (global_binaryWriter = new BinaryWriter(File.Open(sceneFilePath, FileMode.Create)))
        {
            // 방문한 객체들을 추적하기 위한 Set을 초기화
            HashSet<Transform> visitedTransforms = new HashSet<Transform>();

            WriteString("<Hierarchy>:");

            // 첫 번째 자식부터 재귀 호출 시작
            WriteFrameHierarchyInfo(transform, visitedTransforms);

            WriteString("</Hierarchy>");

            // Flush를 통해 데이터를 바로 파일에 기록
            global_binaryWriter.Flush();

            // BinaryWriter는 using 블록을 벗어나면 자동으로 Close()가 호출되므로 명시적으로 Close 호출이 필요하지 않습니다.
        }

        print("Model Binary Write Completed");
    }
}