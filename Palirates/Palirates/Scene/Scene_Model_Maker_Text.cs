using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using UnityEditor;
using System.Text;

public class Scene_Model_Maker_Text : MonoBehaviour
{
    public string sceneFileName = "Scene_Name";

    private List<string> m_rTextureNamesListForCounting = new List<string>();
    private List<string> m_rTextureNamesListForWriting = new List<string>();


    private StreamWriter global_textWriter = null;
    private StreamWriter temp_textWriter = null;

    private int m_nFrames = 0;

    const float EPSILON = 1.0e-6f;

    bool IsZero(float fValue) { return ((Mathf.Abs(fValue) < EPSILON)); }
    bool IsEqual(float fA, float fB) { return (IsZero(fA - fB)); }

    bool FindTextureByName(List<string> textureNamesList, Texture texture)
    {
        if (texture)
        {
            string strTextureName = string.Copy(texture.name).Replace(" ", "_");
            for (int i = 0; i < textureNamesList.Count; i++)
            {
                if (textureNamesList.Contains(strTextureName)) return (true);
            }
            textureNamesList.Add(strTextureName);
            return (false);
        }
        else
        {
            return (true);
        }
    }

    void WriteTabs(int nLevel)
    {
        for (int i = 0; i < nLevel; i++) global_textWriter.Write("\t");
    }

    void WriteObjectName(int nLevel, string strHeader, Object obj)
    {
        WriteTabs(nLevel);
        global_textWriter.Write(strHeader + " ");
        global_textWriter.WriteLine((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteString(string strToWrite)
    {
        global_textWriter.Write(strToWrite);
    }

    void WriteString(int nLevel, string strToWrite)
    {
        WriteTabs(nLevel);
        global_textWriter.Write(strToWrite);
    }

    void WriteLineString(string strToWrite)
    {
        global_textWriter.WriteLine(strToWrite);
    }

    void WriteLineString(int nLevel, string strToWrite)
    {
        WriteTabs(nLevel);
        global_textWriter.WriteLine(strToWrite);
    }

    void WriteTextureName(int nLevel, string strHeader, Texture texture)
    {
        WriteTabs(nLevel);
        global_textWriter.Write(strHeader + " ");
        if (texture)
        {
            if (FindTextureByName(m_rTextureNamesListForWriting, texture))
            {
                global_textWriter.WriteLine("@" + string.Copy(texture.name).Replace(" ", "_"));
            }
            else
            {
                global_textWriter.WriteLine(string.Copy(texture.name).Replace(" ", "_"));
            }
        }
        else
        {
            global_textWriter.WriteLine("null");
        }
    }

    void WriteVector(Vector2 v)
    {
        global_textWriter.Write(v.x + " " + v.y + " ");
    }

    void WriteVector(string strHeader, Vector2 v)
    {
        global_textWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Vector3 v)
    {
        global_textWriter.Write(v.x + " " + v.y + " " + v.z + " ");
    }

    void WriteVector(string strHeader, Vector3 v)
    {
        global_textWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Vector4 v)
    {
        global_textWriter.Write(v.x + " " + v.y + " " + v.z + " " + v.w + " ");
    }

    void WriteVector(string strHeader, Vector4 v)
    {
        global_textWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Quaternion q)
    {
        global_textWriter.Write(q.x + " " + q.y + " " + q.z + " " + q.w + " ");
    }

    void WriteVector(string strHeader, Quaternion q)
    {
        global_textWriter.Write(strHeader);
        WriteVector(q);
    }

    void WriteVectors(int nLevel, string strHeader, Vector2[] vectors)
    {
        WriteString(nLevel, strHeader + " " + vectors.Length + " ");
        if (vectors.Length > 0)
        {
            foreach (Vector2 v in vectors) global_textWriter.Write(v.x + " " + v.y + " ");
        }
        global_textWriter.WriteLine(" ");
    }

    void WriteVectors(int nLevel, string strHeader, Vector3[] vectors)
    {
        WriteString(nLevel, strHeader + " " + vectors.Length + " ");
        if (vectors.Length > 0)
        {
            foreach (Vector3 v in vectors) global_textWriter.Write(v.x + " " + v.y + " " + v.z + " ");
        }
        global_textWriter.WriteLine(" ");
    }

    void WriteVectors(int nLevel, string strHeader, Vector4[] vectors)
    {
        WriteString(nLevel, strHeader + " " + vectors.Length + " ");
        if (vectors.Length > 0)
        {
            foreach (Vector4 v in vectors) global_textWriter.Write(v.x + " " + v.y + " " + v.z + " " + v.w + " ");
        }
        global_textWriter.WriteLine(" ");
    }

    void WriteColors(int nLevel, string strHeader, Color[] colors)
    {
        WriteString(nLevel, strHeader + " " + colors.Length + " ");
        if (colors.Length > 0)
        {
            foreach (Color c in colors) global_textWriter.Write(c.r + " " + c.g + " " + c.b + " " + c.a + " ");
        }
        global_textWriter.WriteLine(" ");
    }

    void WriteTextureCoords(int nLevel, string strHeader, Vector2[] uvs)
    {
        WriteString(nLevel, strHeader + " " + uvs.Length + " ");
        if (uvs.Length > 0)
        {
            foreach (Vector2 uv in uvs) global_textWriter.Write(uv.x + " " + (1.0f - uv.y) + " ");
        }
        global_textWriter.WriteLine(" ");
    }

    void WriteIntegers(int nLevel, string strHeader, int[] integers)
    {
        WriteString(nLevel, strHeader + " " + integers.Length + " ");
        if (integers.Length > 0)
        {
            foreach (int i in integers) global_textWriter.Write(i + " ");
        }
        global_textWriter.WriteLine(" ");
    }

    void WriteMatrix(Matrix4x4 matrix)
    {
        global_textWriter.Write(matrix.m00 + " " + matrix.m10 + " " + matrix.m20 + " " + matrix.m30 + " ");
        global_textWriter.Write(matrix.m01 + " " + matrix.m11 + " " + matrix.m21 + " " + matrix.m31 + " ");
        global_textWriter.Write(matrix.m02 + " " + matrix.m12 + " " + matrix.m22 + " " + matrix.m32 + " ");
        global_textWriter.Write(matrix.m03 + " " + matrix.m13 + " " + matrix.m23 + " " + matrix.m33 + " ");
    }

    void WriteMatrix(Vector3 position, Quaternion rotation, Vector3 scale)
    {
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(position, rotation, scale);
        WriteMatrix(matrix);
    }

    void WriteTransform(int nLevel, string strHeader, Transform current)
    {
        WriteString(nLevel, strHeader + " ");
        WriteVector(current.localPosition);
        WriteVector(current.localEulerAngles);
        WriteVector(current.localScale);
        WriteVector(current.localRotation);
        global_textWriter.WriteLine(" ");
    }

    void WriteLocalMatrix(int nLevel, string strHeader, Transform current)
    {
        WriteString(nLevel, strHeader + " ");
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(current.localPosition, current.localRotation, current.localScale);
        WriteMatrix(matrix);
        global_textWriter.WriteLine(" ");
    }

    void WriteWorldMatrix(int nLevel, string strHeader, Transform current)
    {
        WriteString(nLevel, strHeader + " ");
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(current.position, current.rotation, current.lossyScale);
        WriteMatrix(matrix);
        global_textWriter.WriteLine(" ");
    }
    void WriteMatrixes(int nLevel, string strHeader, Matrix4x4[] matrixes)
    {
        WriteString(nLevel, strHeader + " " + matrixes.Length + " ");
        if (matrixes.Length > 0)
        {
            foreach (Matrix4x4 matrix in matrixes)
            {
                WriteMatrix(matrix);
            }
        }
        global_textWriter.WriteLine(" ");
    }


    int GetTexturesCount(Material[] materials)
    {
        int nTextures = 0;
        for (int i = 0; i < materials.Length; i++)
        {
            if (materials[i].HasProperty("_MainTex"))
            {
                if (!FindTextureByName(m_rTextureNamesListForCounting, materials[i].GetTexture("_MainTex"))) nTextures++;
            }
            if (materials[i].HasProperty("_SpecGlossMap"))
            {
                if (!FindTextureByName(m_rTextureNamesListForCounting, materials[i].GetTexture("_SpecGlossMap"))) nTextures++;
            }
            if (materials[i].HasProperty("_MetallicGlossMap"))
            {
                if (!FindTextureByName(m_rTextureNamesListForCounting, materials[i].GetTexture("_MetallicGlossMap"))) nTextures++;
            }
            if (materials[i].HasProperty("_BumpMap"))
            {
                if (!FindTextureByName(m_rTextureNamesListForCounting, materials[i].GetTexture("_BumpMap"))) nTextures++;
            }
            if (materials[i].HasProperty("_EmissionMap"))
            {
                if (!FindTextureByName(m_rTextureNamesListForCounting, materials[i].GetTexture("_EmissionMap"))) nTextures++;
            }
            if (materials[i].HasProperty("_DetailAlbedoMap"))
            {
                if (!FindTextureByName(m_rTextureNamesListForCounting, materials[i].GetTexture("_DetailAlbedoMap"))) nTextures++;
            }
            if (materials[i].HasProperty("_DetailNormalMap"))
            {
                if (!FindTextureByName(m_rTextureNamesListForCounting, materials[i].GetTexture("_DetailNormalMap"))) nTextures++;
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


    void WriteMaterials(int nLevel, Material[] materials)
    {
        WriteLineString(nLevel, "<Materials>: " + materials.Length);
        if (materials.Length > 0)
        {
            for (int i = 0; i < materials.Length; i++)
            {
                WriteLineString(nLevel + 1, "<Material>: " + i);

                if (materials[i].HasProperty("_Color"))
                {
                    Color albedo = materials[i].GetColor("_Color");
                    WriteLineString(nLevel + 2, "<AlbedoColor>: " + albedo.r + " " + albedo.g + " " + albedo.b + " " + albedo.a);
                }
                if (materials[i].HasProperty("_EmissionColor"))
                {
                    Color emission = materials[i].GetColor("_EmissionColor");
                    WriteLineString(nLevel + 2, "<EmissiveColor>: " + emission.r + " " + emission.g + " " + emission.b + " " + emission.a);
                }
                if (materials[i].HasProperty("_SpecColor"))
                {
                    Color specular = materials[i].GetColor("_SpecColor");
                    WriteLineString(nLevel + 2, "<SpecularColor>: " + specular.r + " " + specular.g + " " + specular.b + " " + specular.a);
                }
                if (materials[i].HasProperty("_Glossiness"))
                {
                    WriteLineString(nLevel + 2, "<Glossiness>: " + materials[i].GetFloat("_Glossiness"));
                }
                if (materials[i].HasProperty("_Smoothness"))
                {
                    WriteLineString(nLevel + 2, "<Smoothness>: " + materials[i].GetFloat("_Smoothness"));
                }
                if (materials[i].HasProperty("_Metallic"))
                {
                    WriteLineString(nLevel + 2, "<Metallic>: " + materials[i].GetFloat("_Metallic"));
                }
                if (materials[i].HasProperty("_SpecularHighlights"))
                {
                    WriteLineString(nLevel + 2, "<SpecularHighlight>: " + materials[i].GetFloat("_SpecularHighlights"));
                }
                if (materials[i].HasProperty("_GlossyReflections"))
                {
                    WriteLineString(nLevel + 2, "<GlossyReflection>: " + materials[i].GetFloat("_GlossyReflections"));
                }
                if (materials[i].HasProperty("_MainTex"))
                {
                    Texture mainAlbedoMap = materials[i].GetTexture("_MainTex");
                    WriteTextureName(nLevel + 2, "<AlbedoMap>:", mainAlbedoMap);
                }
                if (materials[i].HasProperty("_SpecGlossMap"))
                {
                    Texture specularcMap = materials[i].GetTexture("_SpecGlossMap");
                    WriteTextureName(nLevel + 2, "<SpecularMap>:", specularcMap);
                }
                if (materials[i].HasProperty("_MetallicGlossMap"))
                {
                    Texture metallicMap = materials[i].GetTexture("_MetallicGlossMap");
                    WriteTextureName(nLevel + 2, "<MetallicMap>:", metallicMap);
                }
                if (materials[i].HasProperty("_BumpMap"))
                {
                    Texture bumpMap = materials[i].GetTexture("_BumpMap");
                    WriteTextureName(nLevel + 2, "<NormalMap>:", bumpMap);
                }
                if (materials[i].HasProperty("_EmissionMap"))
                {
                    Texture emissionMap = materials[i].GetTexture("_EmissionMap");
                    WriteTextureName(nLevel + 2, "<EmissionMap>:", emissionMap);
                }
                if (materials[i].HasProperty("_DetailAlbedoMap"))
                {
                    Texture detailAlbedoMap = materials[i].GetTexture("_DetailAlbedoMap");
                    WriteTextureName(nLevel + 2, "<DetailAlbedoMap>:", detailAlbedoMap);
                }
                if (materials[i].HasProperty("_DetailNormalMap"))
                {
                    Texture detailNormalMap = materials[i].GetTexture("_DetailNormalMap");
                    WriteTextureName(nLevel + 2, "<DetailNormalMap>:", detailNormalMap);
                }
            }
        }
        WriteLineString(nLevel, "</Materials>");
    }


    void WriteFrameInfo(int nLevel, Transform current)
    {
        int nTextures = GetTexturesCount(current);
        WriteObjectName(nLevel, "<Frame>: " + m_nFrames++ + " " + nTextures, current.gameObject);

        WriteTransform(nLevel + 1, "<Transform>:", current);
        WriteLocalMatrix(nLevel + 1, "<TransformMatrix>:", current);

        MeshFilter meshFilter = current.gameObject.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = current.gameObject.GetComponent<MeshRenderer>();

        if (meshFilter && meshRenderer)
        {
            WriteMeshInfo(0, meshFilter.sharedMesh);

            Material[] materials = meshRenderer.materials;
            if (materials.Length > 0) WriteMaterials(nLevel + 1, materials);
        }
    }

 
    void WriteFrameHierarchyInfo(int nLevel, Transform current, HashSet<Transform> visitedTransforms)
    {
        if (visitedTransforms.Contains(current))
            return;

        visitedTransforms.Add(current);


        WriteFrameInfo(nLevel, current);

        WriteLineString(nLevel + 1, "<Children>: " + current.childCount);

        if (current.childCount > 0)
        {
            for (int k = 0; k < current.childCount; k++) WriteFrameHierarchyInfo(nLevel + 2, current.GetChild(k), visitedTransforms);
        }

        WriteLineString(nLevel, "</Frame>");
    }


    void WriteFrameNameHierarchy(Transform current)
    {
        global_textWriter.Write(string.Copy(current.gameObject.name).Replace(" ", "_") + " ");

        if (current.childCount > 0)
        {
            for (int k = 0; k < current.childCount; k++) WriteFrameNameHierarchy(current.GetChild(k));
        }
    }

    void WriteFrameNames(int nLevel, string strHeader)
    {
        WriteString(nLevel, strHeader);
        WriteFrameNameHierarchy(transform);
        global_textWriter.WriteLine(" ");
    }

    //================================================================================


    void WriteMeshInfo(int nLevel, Mesh mesh)
    {
        string meshFileName = mesh.name.Replace(" ", "_");

        string baseDirectoryPath = Path.Combine(Application.dataPath, "Scene_File", "Meshes");

        //txt 폴더 생성
        string txtDirectoryPath = Path.Combine(baseDirectoryPath, "txt");


        if (!Directory.Exists(txtDirectoryPath))
        {
            Directory.CreateDirectory(txtDirectoryPath);
            Debug.Log("txt 폴더 생성됨.");
        }


        // txt 형식 파일 경로
        string txtFilePath = Path.Combine(txtDirectoryPath, meshFileName + ".txt");

        // 이미 저장된 메시 파일이 존재하면 생략
        if (File.Exists(txtFilePath))
        {
            Debug.Log("Mesh already saved: " + meshFileName);
            return;  // 파일이 이미 존재하면 저장하지 않음
        }

        temp_textWriter = global_textWriter;

        // txt 형식으로 저장
        using (StreamWriter meshWriter = new StreamWriter(File.Open(txtFilePath, FileMode.Create)))
        {
            global_textWriter = meshWriter;

            WriteObjectName(nLevel, "<Mesh>: " + mesh.vertexCount, mesh);

            WriteLineString(nLevel + 1, "<Bounds>: " + mesh.bounds.center.x + " " + mesh.bounds.center.y + " " + mesh.bounds.center.z + " " + mesh.bounds.extents.x + " " + mesh.bounds.extents.y + " " + mesh.bounds.extents.z);

            WriteVectors(nLevel + 1, "<Positions>:", mesh.vertices);
            WriteColors(nLevel + 1, "<Colors>:", mesh.colors);
            WriteTextureCoords(nLevel + 1, "<TextureCoords0>:", mesh.uv);
            WriteTextureCoords(nLevel + 1, "<TextureCoords1>:", mesh.uv2);
            WriteVectors(nLevel + 1, "<Normals>:", mesh.normals);

            if ((mesh.normals.Length > 0) && (mesh.tangents.Length > 0))
            {
                Vector3[] tangents = new Vector3[mesh.tangents.Length];
                Vector3[] bitangents = new Vector3[mesh.tangents.Length];
                for (int i = 0; i < mesh.tangents.Length; i++)
                {
                    tangents[i] = new Vector3(mesh.tangents[i].x, mesh.tangents[i].y, mesh.tangents[i].z);
                    bitangents[i] = Vector3.Normalize(Vector3.Cross(mesh.normals[i], tangents[i])) * mesh.tangents[i].w;
                }

                WriteVectors(nLevel + 1, "<Tangents>:", tangents);
                WriteVectors(nLevel + 1, "<BiTangents>:", bitangents);
            }

            WriteLineString(nLevel + 1, "<SubMeshes>: " + mesh.subMeshCount);
            if (mesh.subMeshCount > 0)
            {
                for (int i = 0; i < mesh.subMeshCount; i++)
                {
                    int[] subindicies = mesh.GetTriangles(i);
                    WriteIntegers(nLevel + 2, "<SubMesh>: " + i, subindicies);
                }
            }

            WriteLineString(nLevel, "</Mesh>");

            global_textWriter = temp_textWriter;

        }

        Debug.Log("Mesh info saved to bin files for: " + meshFileName);
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
        string sceneFilePath = Path.Combine(sceneFileDirectory, sceneFileName + ".txt");

        // binaryWriter를 생성할 때 경로를 정확히 지정
        global_textWriter = new StreamWriter(File.Open(sceneFilePath, FileMode.Create));

        // 방문한 객체들을 추적하기 위한 Set을 초기화
        HashSet<Transform> visitedTransforms = new HashSet<Transform>();

        WriteString("<Hierarchy>:");

        // 첫 번째 자식부터 재귀 호출 시작
        WriteFrameHierarchyInfo(1, transform, visitedTransforms);

        WriteString("</Hierarchy>");

        global_textWriter.Flush();
        global_textWriter.Close();

        print("Model Binary Write Completed");
        print("Model Text Write Completed");
    }


}



