#ifndef vtkMyGPURayCastVolumeMapper_h
#define vtkMyGPURayCastVolumeMapper_h

#include <sstream>

#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPlaneCollection.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkTimerLog.h"
#include "vtkVolumeProperty.h"
#include "vtkTransform.h"
//#include "vtkOpenGLExtensionManager.h"
//#include "vtkgl.h"
#include "vtk_glew.h"

#include "vtkSlicerGPURayCastVolumeMapper.h"

class vtkMyGPURayCastVolumeMapper :
	public vtkSlicerGPURayCastVolumeMapper
{
public:
	vtkTypeMacro(vtkMyGPURayCastVolumeMapper, vtkSlicerGPURayCastVolumeMapper);
	static vtkMyGPURayCastVolumeMapper *New();
	vtkMyGPURayCastVolumeMapper(){}
	virtual ~vtkMyGPURayCastVolumeMapper(){}

	virtual void LoadFragmentShaders()
	{
		std::cout << "vtkMyGPURayCastVolumeMapper::LoadFragmentShaders()" << std::endl;
		std::ostringstream fp_oss;
		fp_oss <<
			"varying vec3 ViewDir;                                                                 \n"
			"uniform sampler3D TextureVol;                                                         \n"
			"uniform sampler3D TextureVol1;                                                        \n"
			"uniform sampler2D TextureColorLookup;                                                 \n"
			"uniform mat4 ParaMatrix;                                                              \n"
			"uniform mat4 VolumeMatrix;                                                            \n"
			"//uniform mat4 ParaMatrix1;                                                             \n"
			"                                                                                      \n"
			"//ParaMatrix:                                                                         \n"
			"//EyePos.x,      EyePos.y,      EyePos.z,     Step                                    \n"
			"//VolBBoxLow.x,  VolBBoxLow.y,  VolBBoxLow.z, VolBBoxHigh.x                           \n"
			"//VolBBoxHigh.y, VolBBoxHigh.z, MinDist,      DepthPeelingThreshold,                  \n"
			"//ICPE_s,        GlobalAlpha,   MinMaxLen,    ICPE_t                                  \n"
			"                                                                                      \n"
			"vec4 computeRayEnd()                                                                  \n"
			"{                                                                                     \n"
			"  vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
			"  vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);              \n"
			"  vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);              \n"
			"  mmn = clamp(mmn, 0.0, 1.0);                                                         \n"
			"  mmx = clamp(mmx, 0.0, 1.0);                                                         \n"
			"                                                                                      \n"
			"  if (all(greaterThanEqual(o, mmn)) && all(lessThanEqual(o, mmx)) )                   \n"
			"    return gl_TexCoord[0];                                                            \n"
			"                                                                                      \n"
			"  vec3 a1 = gl_TexCoord[0].xyz;                                                       \n"
			"  vec3 a2 = a1 + normalize(a1 - o) * length(mmx - mmn);                               \n"
			"  vec3 a3;                                                                            \n"
			"                                                                                      \n"
			"  float halfStep = ParaMatrix[0][3]*0.5;                                              \n"
			"  for (int i = 0; i < 10; i++)                                                        \n"
			"  {                                                                                   \n"
			"    a3 = (a1 + a2) * 0.5;                                                             \n"
			"    if (length(a2 - a1) <= halfStep)                                                  \n"
			"      return vec4(clamp(a3, mmn, mmx), 1.0);                                          \n"
			"    if ( all(greaterThanEqual(a3, mmn)) && all(lessThanEqual(a3, mmx)) )              \n"
			"      a1 = a3;                                                                        \n"
			"    else                                                                              \n"
			"      a2 = a3;                                                                        \n"
			"  }                                                                                   \n"
			"  return vec4(clamp(a3, mmn, mmx), 1.0);                                              \n"
			"}                                                                                     \n"
			"                                                                                      \n"
			"vec4 computeRayOrigin()                                                               \n"
			"{                                                                                     \n"
			"  vec3 o = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);                \n"
			"  vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);              \n"
			"  vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);              \n"
			"  mmn = clamp(mmn, 0.0, 1.0);                                                         \n"
			"  mmx = clamp(mmx, 0.0, 1.0);                                                         \n"
			"                                                                                      \n"
			"  if (all(greaterThanEqual(o, mmn)) && all(lessThanEqual(o, mmx)) )                   \n"
			"    return vec4(o, 1.0);                                                              \n"
			"  else                                                                                \n"
			"    return gl_TexCoord[0];                                                            \n"
			"}                                                                                     \n"
			"                                                                                      \n";
		//color lookup
		switch (this->GetInput()->GetNumberOfScalarComponents())
		{
		case 1:
			fp_oss <<
				"vec4 voxelColor(vec3 coord)                                                          \n"
				"{                                                                                    \n"
				"  vec4 scalar = texture3D(TextureVol, coord);                                        \n"
				"  return texture2D(TextureColorLookup, vec2(scalar.x, scalar.w));                    \n"
				"}                                                                                    \n"
				"                                                                                     \n";
			break;
		case 2:
			fp_oss <<
				"vec4 voxelColor(vec3 coord)                                                          \n"
				"{                                                                                    \n"
				"  vec4 color = vec4(0);                                                              \n"
				"  vec4 scalar = texture3D(TextureVol, coord);                                        \n"
				"  color = texture2D(TextureColorLookup, vec2(scalar.x, scalar.w));                   \n"
				"  vec4 opacity = texture2D(TextureAlphaLookup, vec2(scalar.y, scalar.w));            \n"
				"  color.w = opacity.w;                                                               \n"
				"  return color;                                                                      \n"
				"}                                                                                    \n";
			break;
		case 3:
		case 4:
			fp_oss <<
				"vec4 voxelColor(vec3 coord)                                                          \n"
				"{                                                                                    \n"
				"  vec4 color = vec4(0);                                                              \n"
				"  color = texture3D(TextureVol, coord);                                              \n"
				"  vec4 scalar = texture3D(TextureVol1, coord);                                       \n"
				"  vec4 opacity = texture2D(TextureAlphaLookup, vec2(color.w, scalar.w));             \n"
				"  color.w = opacity.w;                                                               \n"
				"  return color;                                                                      \n"
				"}                                                                                    \n";
			break;
		}

		//scalar lookup
		switch (this->GetInput()->GetNumberOfScalarComponents())
		{
		case 1:
		case 2:
			fp_oss <<
				"float voxelScalar(vec3 coord)                                                        \n"
				"{                                                                                    \n"
				"  return texture3D(TextureVol, coord).x;                                             \n"
				"}                                                                                    \n";
			break;
		case 3:
		case 4:
			fp_oss <<
				"float voxelScalar(vec3 coord)                                                        \n"
				"{                                                                                    \n"
				"  return texture3D(TextureVol, coord).w;                                             \n"
				"}                                                                                    \n";
			break;
		}

		if (this->Technique != 2 && this->Technique != 3) //normal lookup (not needed for MIP and MINIP)
		{
			fp_oss <<
				"vec3 voxelNormal(vec3 coord)                                                           \n"
				"{                                                                                    \n"
				"  {//on-the-fly normal estimation                                                    \n"
				"   //vec4 sample1, sample2;                                                            \n"
				"   //sample1.x = texture3D(TextureVol, coord + vec3(-ParaMatrix1[0][0], 0.0, 0.0)).x;  \n"
				"   //sample1.y = texture3D(TextureVol, coord + vec3(0.0, -ParaMatrix1[0][1], 0.0)).x;  \n"
				"   //sample1.z = texture3D(TextureVol, coord + vec3(0.0, 0.0, -ParaMatrix1[0][2])).x;  \n"
				"   //sample2.x = texture3D(TextureVol, coord + vec3(ParaMatrix1[0][0], 0.0, 0.0)).x;   \n"
				"   //sample2.y = texture3D(TextureVol, coord + vec3(0.0, ParaMatrix1[0][1], 0.0)).x;   \n"
				"   //sample2.z = texture3D(TextureVol, coord + vec3(0.0, 0.0, ParaMatrix1[0][2])).x;   \n"
				"   //vec4 normal = sample1 - sample2;                                                  \n"
				"   //if (length(normal) < 0.001) return vec3(0);                                     \n"
				"   //normal = VolumeMatrix * normalize(normal);                                      \n"
				"   //return gl_NormalMatrix * normal.xyz;                                            \n"
				"  }                                                                                  \n"
				"  {                                                                                  \n"
				"   vec4 normal = texture3D(TextureVol1, coord);                                      \n"
				"   normal = normal * 2.0 - 1.0;                                                      \n"
				"   normal = VolumeMatrix * normal;                                                   \n"
				"   return gl_NormalMatrix * normal.xyz;                                              \n"
				"  }                                                                                  \n"
				"}                                                                                    \n";
		}

		// shading
		if (this->Technique == 0 || this->Technique == 4 || this->Technique == 5)
		{
			fp_oss <<
				"vec4 directionalLight(vec3 coord, vec3 lightDir, vec4 color)                              \n"
				"{                                                                                         \n"
				"  vec3    normal = voxelNormal(coord);                                                    \n"
				"  //if (length(normal) < 0.001)                                                             \n"
				"  //  return gl_FrontMaterial.ambient * color;                                              \n"
				"  normal = normalize(normal);                                                             \n"
				"  float   NdotL = abs(dot(normal, lightDir));                                             \n"
				"  vec4    specular = vec4(0);                                                             \n"
				"  if (NdotL > 0.0)                                                                        \n"
				"  {                                                                                       \n"
				"    float   NdotHV = max( dot( normal, gl_LightSource[0].halfVector.xyz), 0.0);           \n"
				"    specular = (gl_FrontMaterial.specular) * pow(NdotHV, gl_FrontMaterial.shininess)*color;   \n"
				"  }                                                                                           \n"
				"  vec4    diffuse = (gl_FrontMaterial.ambient + gl_FrontMaterial.diffuse * NdotL) * color;    \n"
				"  return (specular + diffuse);                                                                \n"
				"}                                                                                             \n";
		}
		else if (this->Technique == 1)
		{
			fp_oss <<
				"vec4 edgeColoring(vec3 coord, vec4 diffuse)                                              \n"
				"{                                                                                        \n"
				"  vec3    normal = normalize(voxelNormal(coord));                                        \n"
				"  float   NdotV = abs(dot(normal, normalize(-ViewDir)));                                 \n"
				"  return diffuse*NdotV;                                                                  \n"
				"}                                                                                        \n";
		}

		if (this->Technique == 5)
		{
			fp_oss <<
				"float ICPE(vec3 coord, float shading, float alpha, float dist)                           \n"
				"{                                                                                        \n"
				"   float gradMag = texture3D(TextureVol, coord).w;                                       \n"
				"   float base = shading*ParaMatrix[3][3]*(1.0-dist)*(1.0-alpha);                         \n"
				"   if (base > 0.0)                                                                       \n"
				"     return pow(gradMag, pow(base, ParaMatrix[3][0]));                                   \n"
				"   else                                                                                  \n"
				"     return 1.0;                                                                         \n"
				"}                                                                                        \n";
		}

		fp_oss <<
			"void main()                                                                            \n"
			"{                                                                                      \n"
			"  vec4 rayOrigin = computeRayOrigin();                                                 \n"
			"  vec4 rayEnd = computeRayEnd();                                                       \n"
			"  vec3 rayDir = rayEnd.xyz - rayOrigin.xyz;                                            \n"
			"  float rayLen = length(rayDir);                                                       \n"
			"                                                                                       \n"
			" if (rayLen > 1.732)                                                                   \n"
			"   rayLen = 1.732;                                                                     \n"
			"                                                                                       \n"
			"  rayDir = normalize(rayDir);                                                          \n"
			"                                                                                       \n"
			"  //do ray casting                                                                     \n"
			"  vec3 rayStep = rayDir*ParaMatrix[0][3];                                              \n"
			"  vec3 nextRayOrigin = rayOrigin.xyz;                                                  \n"
			"                                                                                       \n"
			"  float fading = 1.0;                                                                  \n"
			"  if (ParaMatrix[3][1] > 0.0001)                                                       \n"
			"  {                                                                                    \n"
			"    vec3 eyePos = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);          \n"
			"    vec3 mmn = vec3(ParaMatrix[1][0], ParaMatrix[1][1], ParaMatrix[1][2]);             \n"
			"    vec3 mmx = vec3(ParaMatrix[1][3], ParaMatrix[2][0], ParaMatrix[2][1]);             \n"
			"    mmn = clamp(mmn, 0.0, 1.0);                                                        \n"
			"    mmx = clamp(mmx, 0.0, 1.0);                                                        \n"
			"                                                                                       \n"
			"    if ( any(lessThanEqual(eyePos, mmn)) || any(greaterThanEqual(eyePos, mmx)) )       \n"
			"    {                                                                                  \n"
			"      float toEyeDist = length(nextRayOrigin - eyePos);                                \n"
			"      fading = 1.0 - (toEyeDist - ParaMatrix[2][2])/ParaMatrix[3][2];                  \n"
			"    }                                                                                  \n"
			"  }                                                                                    \n"
			"                                                                                       \n"
			"  vec4 pixelColor = vec4(0);                                                           \n"
			"  float alpha = 0.0;                                                                   \n"
			"  float t = 0.0;                                                                       \n"
			"  vec3  lightDir = normalize( gl_LightSource[0].position.xyz );                        \n"
			"  float depthPeeling = ParaMatrix[2][3];                                               \n"
			"                                                                                       \n"
			"  while( t < rayLen)                                                                   \n"
			"  {                                                                                    \n"
			"    if ( voxelScalar(nextRayOrigin) >= depthPeeling )                                  \n"
			"      break;                                                                           \n"
			"    t += ParaMatrix[0][3];                                                             \n"
			"    nextRayOrigin += rayStep;                                                          \n"
			"    fading -= ParaMatrix[3][1]*ParaMatrix[0][3];                                       \n"
			"  }                                                                                    \n";

		switch (this->Technique)
		{
		case 0:
			fp_oss <<
				"  while( (t < rayLen) && (alpha < 0.985) )                                          \n"
				"  {                                                                                 \n"
				"    vec4 nextColor = voxelColor(nextRayOrigin);                                     \n"
				"    float tempAlpha = nextColor.w;                                                  \n"
				"    nextColor *= vec4(fading, fading, fading, 1.0);                                 \n"
				"                                                                                    \n"
				"    if (tempAlpha > 0.0)                                                            \n"
				"    {                                                                               \n"
				"      nextColor = directionalLight(nextRayOrigin, lightDir, nextColor);             \n"
				"                                                                                    \n"
				"      tempAlpha = (1.0-alpha)*tempAlpha;                                            \n"
				"      pixelColor += nextColor*tempAlpha;                                            \n"
				"      alpha += tempAlpha;                                                           \n"
				"    }                                                                               \n"
				"                                                                                    \n"
				"    t += ParaMatrix[0][3];                                                          \n"
				"    nextRayOrigin += rayStep;                                                       \n"
				"    fading -= ParaMatrix[3][1]*ParaMatrix[0][3];                                    \n"
				"  }                                                                                 \n"
				"    gl_FragColor = vec4(pixelColor.xyz, alpha);                                     \n"
				"}                                                                                   \n";
			break;
		case 1:
			fp_oss <<
				"  while( (t < rayLen) && (alpha < 0.985) )                                          \n"
				"  {                                                                                 \n"
				"    vec4 nextColor = voxelColor(nextRayOrigin);                                     \n"
				"    float tempAlpha = nextColor.w;                                                  \n"
				"    nextColor *= vec4(fading, fading, fading, 1.0);                                 \n"
				"                                                                                    \n"
				"    if (tempAlpha > 0.0)                                                            \n"
				"    {                                                                               \n"
				"      nextColor = edgeColoring(nextRayOrigin, nextColor);                           \n"
				"      tempAlpha = (1.0-alpha)*tempAlpha;                                            \n"
				"      pixelColor += nextColor*tempAlpha;                                            \n"
				"      alpha += tempAlpha;                                                           \n"
				"    }                                                                               \n"
				"                                                                                    \n"
				"    t += ParaMatrix[0][3];                                                          \n"
				"    nextRayOrigin += rayStep;                                                       \n"
				"    fading -= ParaMatrix[3][1]*ParaMatrix[0][3];                                    \n"
				"  }                                                                                 \n"
				"  gl_FragColor = vec4(pixelColor.xyz, alpha);                                       \n"
				"}                                                                                   \n";
			break;
		case 2:
			fp_oss <<
				"  float maxScalar = voxelScalar(nextRayOrigin);                                     \n"
				"  vec3 maxScalarCoord = nextRayOrigin;                                              \n"
				"  float maxFading = fading;                                                         \n"
				"                                                                                    \n"
				"  while( t < rayLen )                                                               \n"
				"  {                                                                                 \n"
				"    float scalar = voxelScalar(nextRayOrigin);                                      \n"
				"    if (maxScalar < scalar)                                                         \n"
				"    {                                                                               \n"
				"      maxScalar = scalar;                                                           \n"
				"      maxScalarCoord = nextRayOrigin;                                               \n"
				"      maxFading = fading;                                                           \n"
				"    }                                                                               \n"
				"                                                                                    \n"
				"    fading -= ParaMatrix[3][1]*ParaMatrix[0][3];                                    \n"
				"    t += ParaMatrix[0][3];                                                          \n"
				"    nextRayOrigin += rayStep;                                                       \n"
				"  }                                                                                 \n"
				"                                                                                    \n"
				"  pixelColor = voxelColor(maxScalarCoord) * vec4(maxFading, maxFading, maxFading, 1.0);      \n"
				"  alpha = pixelColor.w;                                                             \n"
				"  gl_FragColor = vec4(pixelColor.xyz, alpha);                                       \n"
				"}                                                                                   \n";
			break;
		case 3:
			fp_oss <<
				"  float minScalar = voxelScalar(nextRayOrigin);                                     \n"
				"  vec3 minScalarCoord = nextRayOrigin;                                              \n"
				"  while( t < rayLen )                                                               \n"
				"  {                                                                                 \n"
				"    float scalar = voxelScalar(nextRayOrigin);                                      \n"
				"    if (minScalar > scalar)                                                         \n"
				"    {                                                                               \n"
				"      minScalar = scalar;                                                           \n"
				"      minScalarCoord = nextRayOrigin;                                               \n"
				"    }                                                                               \n"
				"                                                                                    \n"
				"    t += ParaMatrix[0][3];                                                          \n"
				"    nextRayOrigin += rayStep;                                                       \n"
				"  }                                                                                 \n"
				"                                                                                    \n"
				"  pixelColor = voxelColor(minScalarCoord);                                          \n"
				"  alpha = pixelColor.w;                                                             \n"
				"  gl_FragColor = vec4(pixelColor.xyz, alpha);                                       \n"
				"}                                                                                   \n";
			break;
		case 4:
			fp_oss <<
				"  while( (t < rayLen) && (alpha < 0.985) )                                          \n"
				"  {                                                                                 \n"
				"    vec4 nextColor = voxelColor(nextRayOrigin);                                     \n"
				"    float tempAlpha = nextColor.w;                                                  \n"
				"    nextColor *= vec4(fading, fading, fading, 1.0);                                 \n"
				"                                                                                    \n"
				"    if (tempAlpha > 0.0)                                                            \n"
				"    {                                                                               \n"
				"      nextColor = directionalLight(nextRayOrigin, lightDir, nextColor);             \n"
				"                                                                                    \n"
				"      tempAlpha = (1.0-alpha)*tempAlpha*texture3D(TextureVol, nextRayOrigin).w;     \n"
				"      pixelColor += nextColor*tempAlpha;                                            \n"
				"      alpha += tempAlpha;                                                           \n"
				"    }                                                                               \n"
				"                                                                                    \n"
				"    t += ParaMatrix[0][3];                                                          \n"
				"    nextRayOrigin += rayStep;                                                       \n"
				"    fading -= ParaMatrix[3][1]*ParaMatrix[0][3];                                    \n"
				"  }                                                                                 \n"
				"    gl_FragColor = vec4(pixelColor.xyz, alpha);                                     \n"
				"}                                                                                   \n";
			break;
		case 5:
			fp_oss <<
				"  vec3 eyePos = vec3(ParaMatrix[0][0], ParaMatrix[0][1], ParaMatrix[0][2]);         \n"
				"  while( (t < rayLen) && (alpha < 0.985) )                                          \n"
				"  {                                                                                 \n"
				"    vec4 nextColor = voxelColor(nextRayOrigin);                                     \n"
				"    float tempAlpha = nextColor.w;                                                  \n"
				"                                                                                    \n"
				"    if (tempAlpha > 0.0)                                                            \n"
				"    {                                                                               \n"
				"      nextColor = directionalLight(nextRayOrigin, lightDir, nextColor);             \n"
				"      float toEyeDist = (length(nextRayOrigin - eyePos) - ParaMatrix[2][2])/ParaMatrix[3][2];\n"
				"      float icpe = ICPE(nextRayOrigin, length(nextColor), alpha, toEyeDist);        \n"
				"                                                                                    \n"
				"      tempAlpha = (1.0-alpha)*tempAlpha*icpe;                                       \n"
				"      pixelColor += nextColor*tempAlpha;                                            \n"
				"      alpha += tempAlpha;                                                           \n"
				"    }                                                                               \n"
				"                                                                                    \n"
				"    t += ParaMatrix[0][3];                                                          \n"
				"    nextRayOrigin += rayStep;                                                       \n"
				"  }                                                                                 \n"
				"  gl_FragColor = vec4(pixelColor.xyz, alpha);                                       \n"
				"}                                                                                   \n";
			break;
		}

		std::string source = fp_oss.str();
		const char* pSourceText = source.c_str();

		vtkgl::ShaderSource(RayCastFragmentShader, 1, &pSourceText, NULL);
		vtkgl::CompileShader(RayCastFragmentShader);

		GLint result;
		vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::COMPILE_STATUS, &result);

		if (!result)
			printf("Fragment Shader Compile Status: FALSE\n");

		GLint infoLogLen;
		vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::INFO_LOG_LENGTH, &infoLogLen);
		try
		{
			vtkgl::GLchar *pInfoLog = (vtkgl::GLchar*)malloc(sizeof(vtkgl::GLchar)*(infoLogLen + 1));
			vtkgl::GetShaderInfoLog(RayCastFragmentShader, infoLogLen, NULL, pInfoLog);
			printf("%s", pInfoLog);
		}
		catch (...)
		{
		}
	}

	virtual void PrintGLErrorString()
	{
		GLenum error = glGetError();

		switch (error)
		{
		case GL_NO_ERROR: printf("GL_NO_ERROR\n"); break;
		case GL_INVALID_ENUM: printf("GL_INVALID_ENUM\n"); break;
		case GL_INVALID_VALUE: printf("GL_INVALID_VALUE\n"); break;
		case GL_INVALID_OPERATION: printf("GL_INVALID_OPERATION\n"); break;

		case GL_STACK_OVERFLOW: printf("GL_STACK_OVERFLOW\n"); break;
		case GL_STACK_UNDERFLOW: printf("GL_STACK_UNDERFLOW\n"); break;
		case GL_OUT_OF_MEMORY: printf("GL_OUT_OF_MEMORY\n"); break;
		}
	}

	virtual void PrintFragmentShaderInfoLog()
	{
		GLint infoLogLen;
		vtkgl::GetShaderiv(RayCastFragmentShader, vtkgl::INFO_LOG_LENGTH, &infoLogLen);
		try
		{
			vtkgl::GLchar *pInfoLog = (vtkgl::GLchar*)malloc(sizeof(vtkgl::GLchar)*(infoLogLen + 1));
			vtkgl::GetShaderInfoLog(RayCastFragmentShader, infoLogLen, NULL, pInfoLog);
			printf("%s", pInfoLog);
		}
		catch (...)
		{
		}
	}

private:
	vtkMyGPURayCastVolumeMapper(const vtkMyGPURayCastVolumeMapper&);  // Not implemented.
	void operator=(const vtkMyGPURayCastVolumeMapper&);  // Not implemented.
};

#endif // vtkMyGPURayCastVolumeMapper_h
