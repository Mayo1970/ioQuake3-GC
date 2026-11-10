#include "../qcommon/q_shared.h"
#include "../renderer/tr_public.h"

static void Null_Shutdown(qboolean destroyWindow)
{
}

static void Null_BeginRegistration(glconfig_t *config)
{
	memset(config, 0, sizeof(*config));
	config->vidWidth = 640;
	config->vidHeight = 480;
	config->windowAspect = 4.0f / 3.0f;
	Q_strncpyz(config->renderer_string, "null", sizeof(config->renderer_string));
	Q_strncpyz(config->vendor_string, "ioQuake3-GC", sizeof(config->vendor_string));
	Q_strncpyz(config->version_string, "null", sizeof(config->version_string));
}

static qhandle_t Null_RegisterModel(const char *name)
{
	return 0;
}

static qhandle_t Null_RegisterSkin(const char *name)
{
	return 0;
}

static qhandle_t Null_RegisterShader(const char *name)
{
	return 0;
}

static qhandle_t Null_RegisterShaderNoMip(const char *name)
{
	return 0;
}

static void Null_LoadWorld(const char *name)
{
}

static void Null_SetWorldVisData(const byte *vis)
{
}

static void Null_EndRegistration(void)
{
}

static void Null_ClearScene(void)
{
}

static void Null_AddRefEntityToScene(const refEntity_t *re)
{
}

static void Null_AddPolyToScene(qhandle_t hShader, int numVerts,
                                const polyVert_t *verts, int num)
{
}

static int Null_LightForPoint(vec3_t point, vec3_t ambientLight,
                              vec3_t directedLight, vec3_t lightDir)
{
	return 0;
}

static void Null_AddLightToScene(const vec3_t org, float intensity, float r,
                                 float g, float b)
{
}

static void Null_AddAdditiveLightToScene(const vec3_t org, float intensity,
                                         float r, float g, float b)
{
}

static void Null_RenderScene(const refdef_t *fd)
{
}

static void Null_SetColor(const float *rgba)
{
}

static void Null_DrawStretchPic(float x, float y, float w, float h,
                                float s1, float t1, float s2, float t2,
                                qhandle_t hShader)
{
}

static void Null_DrawStretchRaw(int x, int y, int w, int h, int cols, int rows,
                                const byte *data, int client, qboolean dirty)
{
}

static void Null_UploadCinematic(int w, int h, int cols, int rows,
                                 const byte *data, int client, qboolean dirty)
{
}

static void Null_BeginFrame(stereoFrame_t stereoFrame)
{
}

static void Null_EndFrame(int *frontEndMsec, int *backEndMsec)
{
}

static int Null_MarkFragments(int numPoints, const vec3_t *points,
                              const vec3_t projection, int maxPoints,
                              vec3_t pointBuffer, int maxFragments,
                              markFragment_t *fragmentBuffer)
{
	return 0;
}

static int Null_LerpTag(orientation_t *tag, qhandle_t model, int startFrame,
                        int endFrame, float frac, const char *tagName)
{
	return 0;
}

static void Null_ModelBounds(qhandle_t model, vec3_t mins, vec3_t maxs)
{
	VectorClear(mins);
	VectorClear(maxs);
}

static void Null_RegisterFont(const char *fontName, int pointSize,
                              fontInfo_t *font)
{
}

static void Null_RemapShader(const char *oldShader, const char *newShader,
                             const char *offsetTime)
{
}

static qboolean Null_GetEntityToken(char *buffer, int size)
{
	return qfalse;
}

static qboolean Null_inPVS(const vec3_t p1, const vec3_t p2)
{
	return qtrue;
}

static void Null_TakeVideoFrame(int h, int w, byte *captureBuffer,
                                byte *encodeBuffer, qboolean motionJpeg)
{
}

static refexport_t null_refexport = {
	.Shutdown = Null_Shutdown,
	.BeginRegistration = Null_BeginRegistration,
	.RegisterModel = Null_RegisterModel,
	.RegisterSkin = Null_RegisterSkin,
	.RegisterShader = Null_RegisterShader,
	.RegisterShaderNoMip = Null_RegisterShaderNoMip,
	.LoadWorld = Null_LoadWorld,
	.SetWorldVisData = Null_SetWorldVisData,
	.EndRegistration = Null_EndRegistration,
	.ClearScene = Null_ClearScene,
	.AddRefEntityToScene = Null_AddRefEntityToScene,
	.AddPolyToScene = Null_AddPolyToScene,
	.LightForPoint = Null_LightForPoint,
	.AddLightToScene = Null_AddLightToScene,
	.AddAdditiveLightToScene = Null_AddAdditiveLightToScene,
	.RenderScene = Null_RenderScene,
	.SetColor = Null_SetColor,
	.DrawStretchPic = Null_DrawStretchPic,
	.DrawStretchRaw = Null_DrawStretchRaw,
	.UploadCinematic = Null_UploadCinematic,
	.BeginFrame = Null_BeginFrame,
	.EndFrame = Null_EndFrame,
	.MarkFragments = Null_MarkFragments,
	.LerpTag = Null_LerpTag,
	.ModelBounds = Null_ModelBounds,
	.RegisterFont = Null_RegisterFont,
	.RemapShader = Null_RemapShader,
	.GetEntityToken = Null_GetEntityToken,
	.inPVS = Null_inPVS,
	.TakeVideoFrame = Null_TakeVideoFrame,
};

refexport_t *GetRefAPI(int apiVersion, refimport_t *rimp)
{
	if (apiVersion != REF_API_VERSION) {
		rimp->Printf(PRINT_ALL, "GetRefAPI: bad apiVersion %d\n", apiVersion);
		return NULL;
	}

	return &null_refexport;
}
