/* NOTES FOR RAYLIB FOR BETTER COMPATIBILITY:
    - Make the examples more enviroment-independent? 
      Like how you have the projects folder and you can compile using different tools there.


#define RLVK_ENABLE_VULKAN_VALIDATION_LAYER
    for enabling the Vulkan validation layer (it's basically the same as the OpenGL Debug context)

*/


#ifndef RLGL_H
#define RLGL_H

#define RLVK_ENABLE_VULKAN_VALIDATION_LAYER //TODO: Remove!!!!

#define NULL 0 //For some reason the compiler was complaining it can't find NULL so whatever it's here

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.h>

#define RLGL_VERSION  "4.5"

// Function specifiers in case library is build/used as a shared library (Windows)
// NOTE: Microsoft specifiers to tell compiler that symbols are imported/exported from a .dll
#if defined(_WIN32)
    #if defined(BUILD_LIBTYPE_SHARED)
        #define RLAPI __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
    #elif defined(USE_LIBTYPE_SHARED)
        #define RLAPI __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
    #endif
#endif

// Function specifiers definition
#ifndef RLAPI
    #define RLAPI       // Functions defined as 'extern' by default (implicit specifiers)
#endif

// Support TRACELOG macros
#ifndef TRACELOG
    #define TRACELOG(level, ...) (void)0
    #define TRACELOGD(...) (void)0
#endif

// Allow custom memory allocators
#ifndef RL_MALLOC
    #define RL_MALLOC(sz)     malloc(sz)
#endif
#ifndef RL_CALLOC
    #define RL_CALLOC(n,sz)   calloc(n,sz)
#endif
#ifndef RL_REALLOC
    #define RL_REALLOC(n,sz)  realloc(n,sz)
#endif
#ifndef RL_FREE
    #define RL_FREE(p)        free(p)
#endif


// Texture parameters (equivalent to OpenGL defines)
#define RL_TEXTURE_MAG_FILTER                   0x2800      // GL_TEXTURE_MAG_FILTER
#define RL_TEXTURE_MIN_FILTER                   0x2801      // GL_TEXTURE_MIN_FILTER

#define RL_TEXTURE_FILTER_LINEAR                0x2601      // GL_LINEAR

// Matrix modes (equivalent to OpenGL)
#define RL_MODELVIEW                            0x1700      // GL_MODELVIEW
#define RL_PROJECTION                           0x1701      // GL_PROJECTION
#define RL_TEXTURE                              0x1702      // GL_TEXTURE

// Primitive assembly draw modes
#define RL_LINES                                0x0001      // GL_LINES
#define RL_TRIANGLES                            0x0004      // GL_TRIANGLES
#define RL_QUADS                                0x0007      // GL_QUADS

// Trace log level
// NOTE: Organized by priority level
typedef enum {
    RL_LOG_ALL = 0,             // Display all logs
    RL_LOG_TRACE,               // Trace logging, intended for internal use only
    RL_LOG_DEBUG,               // Debug logging, used for internal debugging, it should be disabled on release builds
    RL_LOG_INFO,                // Info logging, used for program execution info
    RL_LOG_WARNING,             // Warning logging, used on recoverable failures
    RL_LOG_ERROR,               // Error logging, used on unrecoverable failures
    RL_LOG_FATAL,               // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    RL_LOG_NONE                 // Disable logging
} rlTraceLogLevel;

// Texture pixel formats
// NOTE: Support depends on OpenGL version
typedef enum {
    RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,        // 8*2 bpp (2 channels)
    RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5,            // 16 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8,            // 24 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,          // 32 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R32,               // 32 bpp (1 channel - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32,         // 32*3 bpp (3 channels - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,      // 32*4 bpp (4 channels - float)
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGB,            // 4 bpp (no alpha)
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA,           // 4 bpp (1 bit alpha)
    RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA,           // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA,           // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC1_RGB,            // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC2_RGB,            // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,       // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGB,            // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA,           // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,       // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA        // 2 bpp
} rlPixelFormat;


// Dynamic vertex buffers (position + texcoords + colors + indices arrays)
typedef struct rlVertexBuffer {
    int elementCount;           // Number of elements in the buffer (QUADS)

    float *vertices;            // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;           // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    unsigned char *colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    unsigned int *indices;      // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    unsigned short *indices;    // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
    unsigned int vaoId;         // OpenGL Vertex Array Object id
    unsigned int vboId[4];      // OpenGL Vertex Buffer Objects id (4 types of vertex data)
} rlVertexBuffer;


// Draw call type
// NOTE: Only texture changes register a new draw, other state-change-related elements are not
// used at this moment (vaoId, shaderId, matrices), raylib just forces a batch draw call if any
// of those state-change happens (this is done in core module)
typedef struct rlDrawCall {
    int mode;                   // Drawing mode: LINES, TRIANGLES, QUADS
    int vertexCount;            // Number of vertex of the draw
    int vertexAlignment;        // Number of vertex required for index alignment (LINES, TRIANGLES)
    //unsigned int vaoId;       // Vertex array id to be used on the draw -> Using RLGL.currentBatch->vertexBuffer.vaoId
    //unsigned int shaderId;    // Shader id to be used on the draw -> Using RLGL.currentShaderId
    unsigned int textureId;     // Texture id to be used on the draw -> Use to create new draw call if changes

    //Matrix projection;        // Projection matrix for this draw -> Using RLGL.projection by default
    //Matrix modelview;         // Modelview matrix for this draw -> Using RLGL.modelview by default
} rlDrawCall;

// rlRenderBatch type
typedef struct rlRenderBatch {
    int bufferCount;            // Number of vertex buffers (multi-buffering support)
    int currentBuffer;          // Current buffer tracking in case of multi-buffering
    rlVertexBuffer *vertexBuffer; // Dynamic buffer(s) for vertex data

    rlDrawCall *draws;          // Draw calls array, depends on textureId
    int drawCounter;            // Draw calls counter
    float currentDepth;         // Current depth value for next draw
} rlRenderBatch;


typedef struct rlvkData {

    bool inited;
    bool vSync;

    //VULKAN DATA
    
    VkInstance vkInstance;
#ifdef RLVK_ENABLE_VULKAN_VALIDATION_LAYER
    VkDebugUtilsMessengerEXT* debugMessenger;
#endif
    VkSurfaceKHR surface;
    VkDevice device;
    VkSwapchainKHR swapChain;
    uint32_t swapChainImageCount;
    VkImageView* swapChainImageViews;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    //-----------

    rlRenderBatch *currentBatch;            // Current render batch
    //rlRenderBatch defaultBatch;             // Default internal render batch

    struct {
        int vertexCounter;                  // Current active render batch vertex counter (generic, used for all batches)
        float texcoordx, texcoordy;         // Current active texture coordinate (added on glVertex*())
        float normalx, normaly, normalz;    // Current active normal (added on glVertex*())
        unsigned char colorr, colorg, colorb, colora;   // Current active color (added on glVertex*())

        int currentMatrixMode;              // Current matrix mode
        Matrix *currentMatrix;              // Current matrix pointer
        Matrix modelview;                   // Default modelview matrix
        Matrix projection;                  // Default projection matrix
        Matrix transform;                   // Transform matrix to be used with rlTranslate, rlRotate, rlScale
        bool transformRequired;             // Require transform matrix application to current draw-call vertex (if required)
        Matrix stack[RL_MAX_MATRIX_STACK_SIZE];// Matrix stack for push/pop
        int stackCounter;                   // Matrix stack counter

        unsigned int defaultTextureId;      // Default texture used on shapes/poly drawing (required by shader)
        unsigned int activeTextureId[RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS];    // Active texture ids to be enabled on batch drawing (0 active by default)
        unsigned int defaultVShaderId;      // Default vertex shader id (used by default shader program)
        unsigned int defaultFShaderId;      // Default fragment shader id (used by default shader program)
        unsigned int defaultShaderId;       // Default shader program id, supports vertex color and diffuse texture
        int *defaultShaderLocs;             // Default shader locations pointer to be used on rendering
        unsigned int currentShaderId;       // Current shader id to be used on rendering (by default, defaultShaderId)
        int *currentShaderLocs;             // Current shader locations pointer to be used on rendering (by default, defaultShaderLocs)

        bool stereoRender;                  // Stereo rendering flag
        Matrix projectionStereo[2];         // VR stereo rendering eyes projection matrices
        Matrix viewOffsetStereo[2];         // VR stereo rendering eyes view offset matrices

        // Blending variables
        int currentBlendMode;               // Blending mode active
        int glBlendSrcFactor;               // Blending source factor
        int glBlendDstFactor;               // Blending destination factor
        int glBlendEquation;                // Blending equation
        int glBlendSrcFactorRGB;            // Blending source RGB factor
        int glBlendDestFactorRGB;           // Blending destination RGB factor
        int glBlendSrcFactorAlpha;          // Blending source alpha factor
        int glBlendDestFactorAlpha;         // Blending destination alpha factor
        int glBlendEquationRGB;             // Blending equation for RGB
        int glBlendEquationAlpha;           // Blending equation for alpha
        bool glCustomBlendModeModified;     // Custom blending factor and equation modification status

        int framebufferWidth;               // Current framebuffer width
        int framebufferHeight;              // Current framebuffer height

    } State;            // Renderer state
    struct {
        bool vao;                           // VAO support (OpenGL ES2 could not support VAO extension) (GL_ARB_vertex_array_object)
        bool instancing;                    // Instancing supported (GL_ANGLE_instanced_arrays, GL_EXT_draw_instanced + GL_EXT_instanced_arrays)
        bool texNPOT;                       // NPOT textures full support (GL_ARB_texture_non_power_of_two, GL_OES_texture_npot)
        bool texDepth;                      // Depth textures supported (GL_ARB_depth_texture, GL_OES_depth_texture)
        bool texDepthWebGL;                 // Depth textures supported WebGL specific (GL_WEBGL_depth_texture)
        bool texFloat32;                    // float textures support (32 bit per channel) (GL_OES_texture_float)
        bool texCompDXT;                    // DDS texture compression support (GL_EXT_texture_compression_s3tc, GL_WEBGL_compressed_texture_s3tc, GL_WEBKIT_WEBGL_compressed_texture_s3tc)
        bool texCompETC1;                   // ETC1 texture compression support (GL_OES_compressed_ETC1_RGB8_texture, GL_WEBGL_compressed_texture_etc1)
        bool texCompETC2;                   // ETC2/EAC texture compression support (GL_ARB_ES3_compatibility)
        bool texCompPVRT;                   // PVR texture compression support (GL_IMG_texture_compression_pvrtc)
        bool texCompASTC;                   // ASTC texture compression support (GL_KHR_texture_compression_astc_hdr, GL_KHR_texture_compression_astc_ldr)
        bool texMirrorClamp;                // Clamp mirror wrap mode supported (GL_EXT_texture_mirror_clamp)
        bool texAnisoFilter;                // Anisotropic texture filtering support (GL_EXT_texture_filter_anisotropic)
        bool computeShader;                 // Compute shaders support (GL_ARB_compute_shader)
        bool ssbo;                          // Shader storage buffer object support (GL_ARB_shader_storage_buffer_object)

        float maxAnisotropyLevel;           // Maximum anisotropy level supported (minimum is 2.0f)
        int maxDepthBits;                   // Maximum bits for depth component

    } ExtSupported;     // Extensions supported flags

} rlvkData;

static rlvkData RLVK = { 0 };


static Matrix rlMatrixIdentity(void)
{
    Matrix result = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    return result;
}

// Get two matrix multiplication
// NOTE: When multiplying matrices... the order matters!
static Matrix rlMatrixMultiply(Matrix left, Matrix right)
{
    Matrix result = { 0 };

    result.m0 = left.m0*right.m0 + left.m1*right.m4 + left.m2*right.m8 + left.m3*right.m12;
    result.m1 = left.m0*right.m1 + left.m1*right.m5 + left.m2*right.m9 + left.m3*right.m13;
    result.m2 = left.m0*right.m2 + left.m1*right.m6 + left.m2*right.m10 + left.m3*right.m14;
    result.m3 = left.m0*right.m3 + left.m1*right.m7 + left.m2*right.m11 + left.m3*right.m15;
    result.m4 = left.m4*right.m0 + left.m5*right.m4 + left.m6*right.m8 + left.m7*right.m12;
    result.m5 = left.m4*right.m1 + left.m5*right.m5 + left.m6*right.m9 + left.m7*right.m13;
    result.m6 = left.m4*right.m2 + left.m5*right.m6 + left.m6*right.m10 + left.m7*right.m14;
    result.m7 = left.m4*right.m3 + left.m5*right.m7 + left.m6*right.m11 + left.m7*right.m15;
    result.m8 = left.m8*right.m0 + left.m9*right.m4 + left.m10*right.m8 + left.m11*right.m12;
    result.m9 = left.m8*right.m1 + left.m9*right.m5 + left.m10*right.m9 + left.m11*right.m13;
    result.m10 = left.m8*right.m2 + left.m9*right.m6 + left.m10*right.m10 + left.m11*right.m14;
    result.m11 = left.m8*right.m3 + left.m9*right.m7 + left.m10*right.m11 + left.m11*right.m15;
    result.m12 = left.m12*right.m0 + left.m13*right.m4 + left.m14*right.m8 + left.m15*right.m12;
    result.m13 = left.m12*right.m1 + left.m13*right.m5 + left.m14*right.m9 + left.m15*right.m13;
    result.m14 = left.m12*right.m2 + left.m13*right.m6 + left.m14*right.m10 + left.m15*right.m14;
    result.m15 = left.m12*right.m3 + left.m13*right.m7 + left.m14*right.m11 + left.m15*right.m15;

    return result;
}


void rlLoadIdentity(void)
{
    *RLVK.State.currentMatrix = rlMatrixIdentity();
}

// Multiply the current matrix by a perspective matrix generated by parameters
void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    Matrix matFrustum = { 0 };

    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(zfar - znear);

    matFrustum.m0 = ((float) znear*2.0f)/rl;
    matFrustum.m1 = 0.0f;
    matFrustum.m2 = 0.0f;
    matFrustum.m3 = 0.0f;

    matFrustum.m4 = 0.0f;
    matFrustum.m5 = ((float) znear*2.0f)/tb;
    matFrustum.m6 = 0.0f;
    matFrustum.m7 = 0.0f;

    matFrustum.m8 = ((float)right + (float)left)/rl;
    matFrustum.m9 = ((float)top + (float)bottom)/tb;
    matFrustum.m10 = -((float)zfar + (float)znear)/fn;
    matFrustum.m11 = -1.0f;

    matFrustum.m12 = 0.0f;
    matFrustum.m13 = 0.0f;
    matFrustum.m14 = -((float)zfar*(float)znear*2.0f)/fn;
    matFrustum.m15 = 0.0f;

    *RLVK.State.currentMatrix = rlMatrixMultiply(*RLVK.State.currentMatrix, matFrustum);
}

// Multiply the current matrix by an orthographic matrix generated by parameters
void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
    // NOTE: If left-right and top-botton values are equal it could create a division by zero,
    // response to it is platform/compiler dependant
    Matrix matOrtho = { 0 };

    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(zfar - znear);

    matOrtho.m0 = 2.0f/rl;
    matOrtho.m1 = 0.0f;
    matOrtho.m2 = 0.0f;
    matOrtho.m3 = 0.0f;
    matOrtho.m4 = 0.0f;
    matOrtho.m5 = 2.0f/tb;
    matOrtho.m6 = 0.0f;
    matOrtho.m7 = 0.0f;
    matOrtho.m8 = 0.0f;
    matOrtho.m9 = 0.0f;
    matOrtho.m10 = -2.0f/fn;
    matOrtho.m11 = 0.0f;
    matOrtho.m12 = -((float)left + (float)right)/rl;
    matOrtho.m13 = -((float)top + (float)bottom)/tb;
    matOrtho.m14 = -((float)zfar + (float)znear)/fn;
    matOrtho.m15 = 1.0f;

    *RLVK.State.currentMatrix = rlMatrixMultiply(*RLVK.State.currentMatrix, matOrtho);
}

// Push the current matrix into RLGL.State.stack
void rlPushMatrix(void)
{
    if (RLVK.State.stackCounter >= RL_MAX_MATRIX_STACK_SIZE) TRACELOG(RL_LOG_ERROR, "RLGL: Matrix stack overflow (RL_MAX_MATRIX_STACK_SIZE)");

    if (RLVK.State.currentMatrixMode == RL_MODELVIEW)
    {
        RLVK.State.transformRequired = true;
        RLVK.State.currentMatrix = &RLVK.State.transform;
    }

    RLVK.State.stack[RLVK.State.stackCounter] = *RLVK.State.currentMatrix;
    RLVK.State.stackCounter++;
}


// Pop latest inserted matrix from RLGL.State.stack
void rlPopMatrix(void)
{
    if (RLVK.State.stackCounter > 0)
    {
        Matrix mat = RLVK.State.stack[RLVK.State.stackCounter - 1];
        *RLVK.State.currentMatrix = mat;
        RLVK.State.stackCounter--;
    }

    if ((RLVK.State.stackCounter == 0) && (RLVK.State.currentMatrixMode == RL_MODELVIEW))
    {
        RLVK.State.currentMatrix = &RLVK.State.modelview;
        RLVK.State.transformRequired = false;
    }
}

// Multiply the current matrix by another matrix
void rlMultMatrixf(const float *matf)
{
    // Matrix creation from array
    Matrix mat = { matf[0], matf[4], matf[8], matf[12],
                   matf[1], matf[5], matf[9], matf[13],
                   matf[2], matf[6], matf[10], matf[14],
                   matf[3], matf[7], matf[11], matf[15] };

    *RLVK.State.currentMatrix = rlMatrixMultiply(*RLVK.State.currentMatrix, mat);
}

// Choose the current matrix to be transformed
void rlMatrixMode(int mode)
{
    if (mode == RL_PROJECTION) RLVK.State.currentMatrix = &RLVK.State.projection;
    else if (mode == RL_MODELVIEW) RLVK.State.currentMatrix = &RLVK.State.modelview;
    //else if (mode == RL_TEXTURE) // Not supported

    RLVK.State.currentMatrixMode = mode;
}

// Multiply the current matrix by a translation matrix
void rlTranslatef(float x, float y, float z)
{
    Matrix matTranslation = {
        1.0f, 0.0f, 0.0f, x,
        0.0f, 1.0f, 0.0f, y,
        0.0f, 0.0f, 1.0f, z,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // NOTE: We transpose matrix with multiplication order
    *RLVK.State.currentMatrix = rlMatrixMultiply(matTranslation, *RLVK.State.currentMatrix);
}

// Multiply the current matrix by a rotation matrix
// NOTE: The provided angle must be in degrees
void rlRotatef(float angle, float x, float y, float z)
{
    Matrix matRotation = rlMatrixIdentity();

    // Axis vector (x, y, z) normalization
    float lengthSquared = x*x + y*y + z*z;
    if ((lengthSquared != 1.0f) && (lengthSquared != 0.0f))
    {
        float inverseLength = 1.0f/sqrtf(lengthSquared);
        x *= inverseLength;
        y *= inverseLength;
        z *= inverseLength;
    }

    // Rotation matrix generation
    float sinres = sinf(DEG2RAD*angle);
    float cosres = cosf(DEG2RAD*angle);
    float t = 1.0f - cosres;

    matRotation.m0 = x*x*t + cosres;
    matRotation.m1 = y*x*t + z*sinres;
    matRotation.m2 = z*x*t - y*sinres;
    matRotation.m3 = 0.0f;

    matRotation.m4 = x*y*t - z*sinres;
    matRotation.m5 = y*y*t + cosres;
    matRotation.m6 = z*y*t + x*sinres;
    matRotation.m7 = 0.0f;

    matRotation.m8 = x*z*t + y*sinres;
    matRotation.m9 = y*z*t - x*sinres;
    matRotation.m10 = z*z*t + cosres;
    matRotation.m11 = 0.0f;

    matRotation.m12 = 0.0f;
    matRotation.m13 = 0.0f;
    matRotation.m14 = 0.0f;
    matRotation.m15 = 1.0f;

    // NOTE: We transpose matrix with multiplication order
    *RLVK.State.currentMatrix = rlMatrixMultiply(matRotation, *RLVK.State.currentMatrix);
}



// Set the viewport area (transformation from normalized device coordinates to window coordinates)
// NOTE: We store current viewport dimensions
void rlViewport(int x, int y, int width, int height)
{
    //TODO
}

// Set current framebuffer width
void rlSetFramebufferWidth(int width)
{
    //TODO
}

// Set current framebuffer height
void rlSetFramebufferHeight(int height)
{
    //TODO
}

#ifdef RLVK_ENABLE_VULKAN_VALIDATION_LAYER
static VKAPI_ATTR VkBool32 VKAPI_CALL rlVulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        return VK_FALSE;
    }

    TRACELOG(LOG_WARNING, "Vulkan: Vulkan debug message: %s", pCallbackData->pMessage);
    TRACELOG(LOG_WARNING, "    > Type: %u", messageType);
    TRACELOG(LOG_WARNING, "    > Severity = %u", messageSeverity);

    return VK_FALSE;
}
#endif


static void rlLoadShaderDefault(void)
{

}

//TODO: These are ugly hacks
const char** glfwGetRequiredInstanceExtensions(uint32_t* count);

void rlglInit(int width, int height, struct GLFWwindow* windowHandle)
{
    VkApplicationInfo appInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Raylib Application", //TODO: Get auctual app name here? Set to null? Keep it like this?
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Raylib",
        .engineVersion = VK_MAKE_VERSION(4, 5, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    uint32_t vulkanExtensionCount = 0;
    char** glfwExtensions = (char**)glfwGetRequiredInstanceExtensions(&vulkanExtensionCount);
    char** vulkanExtensions = RL_MALLOC(vulkanExtensionCount * sizeof(char*));

    for (uint32_t i = 0; i < vulkanExtensionCount; ++i)
    {
        vulkanExtensions[i] = glfwExtensions[i];
    }

    VkInstanceCreateFlags createInfoFlags = 0;

#ifdef PLATFORM_APPLE
    //According to https://vulkan.lunarg.com/doc/sdk/1.3.216.0/mac/getting_started.html
    //Beginning with the 1.3.216 Vulkan SDK, the VK_KHR_PORTABILITY_subset extension is mandatory for MoltenVK (Vulkan porting to metal)
    vulkanExtensions = RL_REALLOC(vulkanExtensions, ++vulkanExtensionCount * sizeof(char*));
    vulkanExtensions[vulkanExtensionCount - 1] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    createInfoFlags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    //TODO: Maybe print available Vulkan extensions? Raylib likes printing stuff like this to the console

#ifdef RLVK_ENABLE_VULKAN_VALIDATION_LAYER
    vulkanExtensions = RL_REALLOC(vulkanExtensions, ++vulkanExtensionCount * sizeof(char*));
    vulkanExtensions[vulkanExtensionCount - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    
    //TODO: Check for validation layer support?

    char* validationLayers[] =
    {
        "VK_LAYER_KHRONOS_validation"
    };
    
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = rlVulkanDebugCallback,
        .pUserData = 0 // Optional
    };
#endif

    VkInstanceCreateInfo instanceCreateInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = vulkanExtensionCount,
        .ppEnabledExtensionNames = vulkanExtensions,
        .flags = createInfoFlags,
#ifdef RLVK_ENABLE_VULKAN_VALIDATION_LAYER
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = validationLayers,
        .pNext = &debugMessengerCreateInfo
#else
        .enabledLayerCount = 0
#endif
    };

    if (vkCreateInstance(&instanceCreateInfo, 0, &RLVK.vkInstance) != VK_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Failed to initialize Instance");
        return;
    }

    RL_FREE(vulkanExtensions);
    
#ifdef RLVK_ENABLE_VULKAN_VALIDATION_LAYER
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = 
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(RLVK.vkInstance, "vkCreateDebugUtilsMessengerEXT");

    if (vkCreateDebugUtilsMessengerEXT == 0)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Failed to get vkCreateDebugUtilsMessengerEXT. Vulkan validation layers are probably not supported on this machine.");
        return;
    }
    
    if (vkCreateDebugUtilsMessengerEXT(RLVK.vkInstance, &debugMessengerCreateInfo, 0, &RLVK.debugMessenger) != VK_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Failed to initialize Debug Messenger");
        return;
    }
#endif

    //Platform dependent surface creation
    if (glfwCreateWindowSurface(RLVK.vkInstance, windowHandle, 0, &RLVK.surface) != VK_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "GLFW, Vulkan: Failed to create a window surface");
        return;
    }

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t graphicsFamily = 0;
    uint32_t presentFamily = 0;

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(RLVK.vkInstance, &physicalDeviceCount, 0);

    if (physicalDeviceCount == 0)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Could not find any GPU with Vulkan support");
        return;
    }

    uint32_t requiredDeviceExtensionCount = 1;
    char* requiredDeviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    //We can only get the devices from Vulkan. There is no feature to let Vulkan pick a device.
    //So we will use a custom scoring system to pick the best device.
    {
        uint32_t* physicaDeviceGraphicsFamilies = RL_MALLOC(physicalDeviceCount * sizeof(uint32_t));
        uint32_t* physicaDevicePresentFamilies = RL_MALLOC(physicalDeviceCount * sizeof(uint32_t));
        int32_t* physicaDeviceScores = RL_MALLOC(physicalDeviceCount * sizeof(int32_t));
        VkPhysicalDevice* physicalDevices = RL_MALLOC(physicalDeviceCount * sizeof(VkPhysicalDevice));
        vkEnumeratePhysicalDevices(RLVK.vkInstance, &physicalDeviceCount, physicalDevices);

        for (uint32_t i = 0; i < physicalDeviceCount; ++i)
        {
            VkPhysicalDevice* device = &physicalDevices[i];

            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(*device, &deviceProperties);

            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(*device, &deviceFeatures);

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, 0);

            VkQueueFamilyProperties* queueFamilies = RL_MALLOC(queueFamilyCount * sizeof(VkQueueFamilyProperties));
            vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, queueFamilies);

            bool foundGraphicsFamily = false;
            bool foundPresentFamily = false;

            for (uint32_t j = 0; j < queueFamilyCount; ++j)
            {
                VkQueueFamilyProperties* queueFamily = &queueFamilies[j];

                if (queueFamily->queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    foundGraphicsFamily = true;
                    physicaDeviceGraphicsFamilies[i] = j;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(*device, j, RLVK.surface, &presentSupport);

                if (presentSupport)
                {
                    foundPresentFamily = true;
                    physicaDevicePresentFamilies[i] = j;
                }
                
                if (foundPresentFamily && foundGraphicsFamily)
                {
                    break;
                }
            }

            RL_FREE(queueFamilies);

            if (!foundGraphicsFamily || !foundPresentFamily)
            {
                physicaDeviceScores[i] = 0;
                continue;
            }

            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(*device, 0, &extensionCount, 0);

            VkExtensionProperties* availableExtensions = RL_MALLOC(extensionCount * sizeof(VkExtensionProperties));
            vkEnumerateDeviceExtensionProperties(*device, 0, &extensionCount, availableExtensions);

            bool* requiredExtensionAvailables = RL_MALLOC(requiredDeviceExtensionCount * sizeof(bool));

            for (uint32_t j = 0; j < requiredDeviceExtensionCount; ++j)
            {
                requiredExtensionAvailables[j] = false;
            }
            
            for (uint32_t j = 0; j < extensionCount; ++j)
            {
                for (uint32_t k = 0; k < requiredDeviceExtensionCount; ++k)
                {
                    if (strcmp(availableExtensions[j].extensionName, requiredDeviceExtensions[k]) == 0)
                    {
                        requiredExtensionAvailables[k] = true;
                    }
                }
            }
            
            bool extensionsSupported = true;

            for (uint32_t j = 0; j < requiredDeviceExtensionCount; ++j)
            {
                if (!requiredExtensionAvailables[j])
                {
                    extensionsSupported = false;
                    break;
                }
            }

            if (!extensionsSupported)
            {
                physicaDeviceScores[i] = 0;
                continue;
            }
            
            uint32_t surfaceFormatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(*device, RLVK.surface, &surfaceFormatCount, 0);
            
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(*device, RLVK.surface, &presentModeCount, 0);

            if (surfaceFormatCount == 0 || presentModeCount == 0)
            {
                physicaDeviceScores[i] = 0;
                continue;
            }

            physicaDeviceScores[i] = 1; //Initalize memory to 1, since 0 means the device is not suitable
            
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                physicaDeviceScores[i] += 1000;
            }

            //Prefer devices where the same queue family supports drawing and presentation,
            //since these have a significantly higher performance (significantly does not mean a lot here, it means significant)
            if (physicaDeviceGraphicsFamilies[i] == physicaDevicePresentFamilies[i])
            {
                physicaDeviceScores[i] += 1000;
            }

            physicaDeviceScores[i] += deviceProperties.limits.maxImageDimension2D;
        }
        
        int32_t highestScore = 0;

        for (uint32_t i = 0; i < physicalDeviceCount; ++i)
        {
            if (physicaDeviceScores[i] > highestScore)
            {
                highestScore = physicaDeviceScores[i];
                physicalDevice = physicalDevices[i];
                graphicsFamily = physicaDeviceGraphicsFamilies[i];
                presentFamily = physicaDevicePresentFamilies[i];
            }
        }

        RL_FREE(physicalDevices);
        RL_FREE(physicaDeviceScores);
        RL_FREE(physicaDevicePresentFamilies);
        RL_FREE(physicaDeviceGraphicsFamilies);
    }

    if (physicalDevice == 0)
    {
        TRACELOG(LOG_WARNING, "RLVK: Could not find a suitable physical device");
        return;
    }
    
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo graphicsQueueCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    VkDeviceQueueCreateInfo presentQueueCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = presentFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };
    
    VkDeviceQueueCreateInfo queueCreateInfos[] =
    {
        graphicsQueueCreateInfo,
        presentQueueCreateInfo
    };

    //If the present family and the graphics family the same we don't need to create seperate queues,
    //because the families are not seperate
    uint32_t queuesToCreate = presentFamily == graphicsFamily ? 1 : 2;

    VkPhysicalDeviceFeatures deviceFeatures = { 0 };

    VkDeviceCreateInfo deviceCreateInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = queuesToCreate,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount = requiredDeviceExtensionCount,
        .ppEnabledExtensionNames = requiredDeviceExtensions,
#ifdef RLVK_ENABLE_VULKAN_VALIDATION_LAYER
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = validationLayers
#else
        .enabledLayerCount = 0
#endif
    };
    
    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, 0, &RLVK.device) != VK_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Failed to create a logical device");
        return;
    }

    VkQueue graphicsQueue;
    vkGetDeviceQueue(RLVK.device, graphicsFamily, 0, &graphicsQueue);

    VkQueue presentQueue;
    vkGetDeviceQueue(RLVK.device, presentFamily, 0, &presentQueue);

    VkExtent2D swapExtent;
    VkFormat swapChainImageFormat;
    
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, RLVK.surface, &capabilities);

        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            swapExtent = capabilities.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };

            //Clamp
            if (actualExtent.width < capabilities.minImageExtent.width)
            {
                actualExtent.width = capabilities.minImageExtent.width;
            }

            if (actualExtent.width > capabilities.maxImageExtent.width)
            {
                actualExtent.width = capabilities.maxImageExtent.width;
            }

            //Clamp
            if (actualExtent.height < capabilities.minImageExtent.height)
            {
                actualExtent.height = capabilities.minImageExtent.height;
            }

            if (actualExtent.width > capabilities.maxImageExtent.height)
            {
                actualExtent.height = capabilities.maxImageExtent.height;
            }

            swapExtent = actualExtent;
        }

        RLVK.swapChainImageCount = capabilities.minImageCount + 1;

        if (capabilities.maxImageCount > 0 && RLVK.swapChainImageCount > capabilities.maxImageCount)
        {
            RLVK.swapChainImageCount = capabilities.maxImageCount;
        }

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, RLVK.surface, &formatCount, 0);

        VkSurfaceFormatKHR surfaceFormat;
        VkSurfaceFormatKHR* formats = RL_MALLOC(formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, RLVK.surface, &formatCount, formats);

        bool foundAvailableFormat = false;

        for (uint32_t i = 0; i < formatCount; ++i)
        {
            VkSurfaceFormatKHR* availableFormat = &formats[i];

            if (availableFormat->format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                foundAvailableFormat = true;
                surfaceFormat = *availableFormat;
                break;
            }
        }

        if (!foundAvailableFormat)
        {
            surfaceFormat = formats[0];
        }

        RL_FREE(formats);

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, RLVK.surface, &presentModeCount, 0);

        VkPresentModeKHR presentMode;
        VkPresentModeKHR* presentModes = 0;
        presentModes = RL_MALLOC(presentModeCount * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, RLVK.surface, &presentModeCount, presentModes);

        /* Note for people that are new to Vulkan:
            - VK_PRESENT_MODE_IMMEDIATE_KHR: No VSync
            - VK_PRESENT_MODE_FIFO_KHR: There is a queue of the drawn frames and the GPU will just display those frames in VSync timing,
                if the queue is full, the CPU is blocked and it has to wait for the GPU
            - VK_PRESENT_MODE_MAILBOX_KHR: There is a queue of the drawn frames which the GPU will display in a VSync timing,
                if the queue is full, the oldest frame is discraded and the newest frame is inserted

            VK_PRESENT_MODE_FIFO_KHR is guarenteed to be available on every system.
        */

        bool foundAvailablePresentMode = false;

        for (uint32_t i = 0; i < presentModeCount; ++i)
        {
            VkPresentModeKHR availablePresentMode = presentModes[i];

            if (RLVK.vSync && availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = availablePresentMode;
                break;
            }

            if (!RLVK.vSync && availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                presentMode = availablePresentMode;
                break;
            }
        }
        
        if (!foundAvailablePresentMode)
        {
            presentMode = VK_PRESENT_MODE_FIFO_KHR;
        }
        
        RL_FREE(presentModes);

        swapChainImageFormat = surfaceFormat.format;

        VkSwapchainCreateInfoKHR createInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = RLVK.surface,
            .minImageCount = RLVK.swapChainImageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = swapExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };
        
        uint32_t queueFamilyIndices[] = { graphicsFamily, presentFamily };

        if (graphicsFamily != presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = 0;
        }

        if (vkCreateSwapchainKHR(RLVK.device, &createInfo, 0, &RLVK.swapChain) != VK_SUCCESS)
        {
            TRACELOG(LOG_WARNING, "Vulkan: Failed to create a swap chain");
            return;
        }
    }

    vkGetSwapchainImagesKHR(RLVK.device, RLVK.swapChain, &RLVK.swapChainImageCount, 0);

    VkImage* swapChainImages = RL_MALLOC(RLVK.swapChainImageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(RLVK.device, RLVK.swapChain, &RLVK.swapChainImageCount, swapChainImages);

    RLVK.swapChainImageViews = RL_MALLOC(RLVK.swapChainImageCount * sizeof(VkImageView));

    for (uint32_t i = 0; i < RLVK.swapChainImageCount; ++i)
    {
        VkImageViewCreateInfo createInfo =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapChainImageFormat,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };

        if (vkCreateImageView(RLVK.device, &createInfo, 0, &RLVK.swapChainImageViews[i]) != VK_SUCCESS)
        {
            TRACELOG(LOG_WARNING, "Vulkan: Failed to create an image view");
            return;
        }
    }

    VkAttachmentDescription colorAttachment =
    {
        .format = swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef =
    {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass =
    {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef
    };

    VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass
    };

    if (vkCreateRenderPass(RLVK.device, &renderPassInfo, 0, &RLVK.renderPass) != VK_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Failed to create render pass");
        return;
    }
    
    const char defaultVShaderCode[] =
    "#version 450                       \n"
    "vec2 positions[3] = vec2[]         \n"
    "(                                  \n"
    "   vec2(0.0, -0.5),                \n"
    "   vec2(0.5, 0.5),                 \n"
    "   vec2(-0.5, 0.5)                 \n"
    ");                                 \n"
    "void main()                        \n"
    "{                                  \n"
    "    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0); \n"
    "}\n\0";

    const char defaultFShaderCode[] =
    "#version 450                       \n"
    "layout(location = 0) out vec4 outColor; \n"
    "void main()                        \n"
    "{                                  \n"
    "    outColor = vec4(1.0, 0.0, 0.0, 1.0); \n"
    "}\n\0";

    shaderc_compiler_t shaderCompiler = shaderc_compiler_initialize();
    shaderc_compile_options_t shaderCompileOptions = shaderc_compile_options_initialize();

    //TODO: Set compile options

    //TODO: Preprocess shader text?

    shaderc_compilation_result_t vsCompileResult =
        shaderc_compile_into_spv(shaderCompiler, defaultVShaderCode, strlen(defaultVShaderCode), shaderc_vertex_shader, 
        "DefaultVertexShader", "main", shaderCompileOptions);

    if (shaderc_result_get_compilation_status(vsCompileResult) != shaderc_compilation_status_success)
    {
        TRACELOG(LOG_WARNING, "RLVK: Failed to compile default vertex shader: %s", shaderc_result_get_error_message(vsCompileResult));
        return;
    }

    size_t vsSpirVLength = shaderc_result_get_length(vsCompileResult);
    uint32_t* vsSpirvVBinary = (uint32_t*)shaderc_result_get_bytes(vsCompileResult);

    VkShaderModuleCreateInfo vertexShaderCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = vsSpirVLength,
        .pCode = vsSpirvVBinary
    };

    VkShaderModule vertexShaderModule;
    if (vkCreateShaderModule(RLVK.device, &vertexShaderCreateInfo, 0, &vertexShaderModule) != VK_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Failed to create vertex shader module");
        return;
    }

    shaderc_result_release(vsCompileResult);

    shaderc_compilation_result_t fsCompileResult =
        shaderc_compile_into_spv(shaderCompiler, defaultFShaderCode, strlen(defaultFShaderCode), shaderc_fragment_shader, 
        "DefaultFragmentShader", "main", shaderCompileOptions);

    if (shaderc_result_get_compilation_status(fsCompileResult) != shaderc_compilation_status_success)
    {
        TRACELOG(LOG_WARNING, "RLVK: Failed to compile default fragment shader: %s", shaderc_result_get_error_message(fsCompileResult));
        return;
    }

    size_t fsSpirVLength = shaderc_result_get_length(fsCompileResult);
    uint32_t* fsSpirvVBinary = (uint32_t*)shaderc_result_get_bytes(fsCompileResult);

    VkShaderModuleCreateInfo fragmentShaderCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = fsSpirVLength,
        .pCode = fsSpirvVBinary
    };

    VkShaderModule fragmentShaderModule;
    if (vkCreateShaderModule(RLVK.device, &fragmentShaderCreateInfo, 0, &fragmentShaderModule) != VK_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Failed to create fragment shader module");
        return;
    }
    
    shaderc_result_release(fsCompileResult);

    shaderc_compile_options_release(shaderCompileOptions);
    shaderc_compiler_release(shaderCompiler);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertexShaderModule,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragmentShaderModule,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

    VkDynamicState dynamicStates[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]),
        .pDynamicStates = dynamicStates
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = 0, //Optional
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = 0 //Optional
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)swapExtent.width,
        .height = (float)swapExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor =
    {
        .offset = { 0, 0 },
        .extent = swapExtent
    };

    VkPipelineViewportStateCreateInfo viewportState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterizer =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f, //Optional
        .depthBiasClamp = 0.0f, //Optional
        .depthBiasSlopeFactor = 0.0f //Optional
    };

    VkPipelineMultisampleStateCreateInfo multisampling =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f, //Optional
        .pSampleMask = 0, //Optional
        .alphaToCoverageEnable = VK_FALSE, //Optional
        .alphaToOneEnable = VK_FALSE //Optional
    };
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment =
    {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD
    };

    VkPipelineColorBlendStateCreateInfo colorBlending =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, //Optional
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants[0] = 0.0f, //Optional
        .blendConstants[1] = 0.0f, //Optional
        .blendConstants[2] = 0.0f, //Optional
        .blendConstants[3] = 0.0f, //Optional
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0, //Optional
        .pSetLayouts = 0, //Optional
        .pushConstantRangeCount = 0, //Optional
        .pPushConstantRanges = 0, //Optional
    };

    if (vkCreatePipelineLayout(RLVK.device, &pipelineLayoutInfo, 0, &RLVK.pipelineLayout) != VK_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Failed to create pipeline layout");
        return;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = 0, //Optional
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = RLVK.pipelineLayout,
        .renderPass = RLVK.renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE, //Optional
        .basePipelineIndex = -1 //Optional
    };

    if (vkCreateGraphicsPipelines(RLVK.device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &RLVK.graphicsPipeline) != VK_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Failed to create graphics pipeline");
        return;
    }

    vkDestroyShaderModule(RLVK.device, vertexShaderModule, 0);
    vkDestroyShaderModule(RLVK.device, fragmentShaderModule, 0);
    

    RLVK.inited = true;
}

void rlglClose(void)
{
    vkDestroyPipeline(RLVK.device, RLVK.graphicsPipeline, 0);
    vkDestroyPipelineLayout(RLVK.device, RLVK.pipelineLayout, 0);
    vkDestroyRenderPass(RLVK.device, RLVK.renderPass, 0);

    for (uint32_t i = 0; i < RLVK.swapChainImageCount; ++i)
    {
        vkDestroyImageView(RLVK.device, RLVK.swapChainImageViews[i], 0);
    }
    
    vkDestroySwapchainKHR(RLVK.device, RLVK.swapChain, 0);
    vkDestroySurfaceKHR(RLVK.vkInstance, RLVK.surface, 0);
    vkDestroyDevice(RLVK.device, 0);

#ifdef RLVK_ENABLE_VULKAN_VALIDATION_LAYER
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = 
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(RLVK.vkInstance, "vkDestroyDebugUtilsMessengerEXT");

    if (vkDestroyDebugUtilsMessengerEXT == 0)
    {
        TRACELOG(LOG_WARNING, "Vulkan: Failed to get vkDestroyDebugUtilsMessengerEXT. Vulkan validation layers are probably not supported on this machine.");
    }

    vkDestroyDebugUtilsMessengerEXT(RLVK.vkInstance, RLVK.debugMessenger, 0);
#endif
    
    vkDestroyInstance(RLVK.vkInstance, 0);
}

void rlLoadExtensions(void *loader)
{
    //Stub for compatibility
}

void rlTextureParameters(unsigned int id, int param, int value)
{
    //TODO
}


void rlEnableDepthTest(void)
{
    //TODO
}

void rlDisableDepthTest(void)
{
    //TODO
}

void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    //TODO: Set clear color
}


void rlClearScreenBuffers(void)
{
    //TODO: Auctally clear the screen
}


void rlDrawRenderBatchActive(void)
{
    //TODO
}

unsigned char *rlReadScreenPixels(int width, int height)
{

}

// Enable rendering to texture (fbo)
void rlEnableFramebuffer(unsigned int id)
{
    //TODO
}

// Disable rendering to texture
void rlDisableFramebuffer(void)
{
    //TODO
}


// Set shader currently active (id and locations)
void rlSetShader(unsigned int id, int *locs)
{
    //TODO
}

// Get default shader locs
int *rlGetShaderLocsDefault(void)
{
    //TODO: What is this exactly?
    int *locs = NULL;
    locs = RLVK.State.defaultShaderLocs;
    return locs;
}

// Get default shader id
unsigned int rlGetShaderIdDefault(void)
{
    unsigned int id = 0;
    id = RLVK.State.defaultShaderId;
    return id;
}

// Shaders management
//-----------------------------------------------------------------------------------------------
// Load shader from code strings
// NOTE: If shader string is NULL, using default vertex/fragment shaders
unsigned int rlLoadShaderCode(const char *vsCode, const char *fsCode)
{
    //TODO
}

int rlGetLocationAttrib(unsigned int shaderId, const char *attribName)
{
    //TODO
}

int rlGetLocationUniform(unsigned int shaderId, const char *uniformName)
{
    //TODO
}

void rlUnloadShaderProgram(unsigned int id)
{
    //TODO
}

void rlEnableShader(unsigned int id)
{
    //TODO
}

void rlSetUniform(int locIndex, const void *value, int uniformType, int count)
{
    //TODO
}

void rlSetUniformMatrix(int locIndex, Matrix mat)
{
    //TODO
}

void rlSetUniformSampler(int locIndex, unsigned int textureId)
{
    //TODO
}



void rlSetBlendMode(int mode)
{
    //TODO
}


// Enable scissor test
void rlEnableScissorTest(void)
{
    //TODO
}

// Scissor test
void rlScissor(int x, int y, int width, int height)
{
    //TODO
    //glScissor(x, y, width, height);
}

// Disable scissor test
void rlDisableScissorTest(void)
{
    //TODO
}

// Enable stereo rendering
void rlEnableStereoRender(void)
{
    RLVK.State.stereoRender = true;
}

// Set eyes projection matrices for stereo rendering
void rlSetMatrixProjectionStereo(Matrix right, Matrix left)
{
    RLVK.State.projectionStereo[0] = right;
    RLVK.State.projectionStereo[1] = left;
}

// Set eyes view offsets matrices for stereo rendering
void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left)
{
    RLVK.State.viewOffsetStereo[0] = right;
    RLVK.State.viewOffsetStereo[1] = left;
}

// Disable stereo rendering
void rlDisableStereoRender(void)
{
    RLVK.State.stereoRender = false;
}


void rlSetTexture(unsigned int id)
{
    //TODO
}



// Draw render batch
// NOTE: We require a pointer to reset batch and increase current buffer (multi-buffer)
void rlDrawRenderBatch(rlRenderBatch *batch)
{
    
}

// Check internal buffer overflow for a given number of vertex
// and force a rlRenderBatch draw call if required
bool rlCheckRenderBatchLimit(int vCount)
{
    bool overflow = false;

    if ((RLVK.State.vertexCounter + vCount) >=
        (RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].elementCount*4))
    {
        overflow = true;

        // Store current primitive drawing mode and texture id
        int currentMode = RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].mode;
        int currentTexture = RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].textureId;

        rlDrawRenderBatch(RLVK.currentBatch);    // NOTE: Stereo rendering is checked inside

        // Restore state of last batch so we can continue adding vertices
        RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].mode = currentMode;
        RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].textureId = currentTexture;
    }

    return overflow;
}


void rlBegin(int mode)
{

}

// Define one vertex (normal)
// NOTE: Normals limited to TRIANGLES only?
void rlNormal3f(float x, float y, float z)
{
    RLVK.State.normalx = x;
    RLVK.State.normaly = y;
    RLVK.State.normalz = z;
}

// Define one vertex (color)
void rlColor4ub(unsigned char x, unsigned char y, unsigned char z, unsigned char w)
{
    RLVK.State.colorr = x;
    RLVK.State.colorg = y;
    RLVK.State.colorb = z;
    RLVK.State.colora = w;
}

// Define one vertex (texture coordinate)
// NOTE: Texture coordinates are limited to QUADS only
void rlTexCoord2f(float x, float y)
{
    RLVK.State.texcoordx = x;
    RLVK.State.texcoordy = y;
}

// Define one vertex (position)
// NOTE: Vertex position data is the basic information required for drawing
void rlVertex3f(float x, float y, float z)
{
    float tx = x;
    float ty = y;
    float tz = z;

    // Transform provided vector if required
    if (RLVK.State.transformRequired)
    {
        tx = RLVK.State.transform.m0*x + RLVK.State.transform.m4*y + RLVK.State.transform.m8*z + RLVK.State.transform.m12;
        ty = RLVK.State.transform.m1*x + RLVK.State.transform.m5*y + RLVK.State.transform.m9*z + RLVK.State.transform.m13;
        tz = RLVK.State.transform.m2*x + RLVK.State.transform.m6*y + RLVK.State.transform.m10*z + RLVK.State.transform.m14;
    }

    // WARNING: We can't break primitives when launching a new batch.
    // RL_LINES comes in pairs, RL_TRIANGLES come in groups of 3 vertices and RL_QUADS come in groups of 4 vertices.
    // We must check current draw.mode when a new vertex is required and finish the batch only if the draw.mode draw.vertexCount is %2, %3 or %4
    if (RLVK.State.vertexCounter > (RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].elementCount*4 - 4))
    {
        if ((RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].mode == RL_LINES) &&
            (RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].vertexCount%2 == 0))
        {
            // Reached the maximum number of vertices for RL_LINES drawing
            // Launch a draw call but keep current state for next vertices comming
            // NOTE: We add +1 vertex to the check for security
            rlCheckRenderBatchLimit(2 + 1);
        }
        else if ((RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].mode == RL_TRIANGLES) &&
            (RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].vertexCount%3 == 0))
        {
            rlCheckRenderBatchLimit(3 + 1);
        }
        else if ((RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].mode == RL_QUADS) &&
            (RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].vertexCount%4 == 0))
        {
            rlCheckRenderBatchLimit(4 + 1);
        }
    }

    // Add vertices
    RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].vertices[3*RLVK.State.vertexCounter] = tx;
    RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].vertices[3*RLVK.State.vertexCounter + 1] = ty;
    RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].vertices[3*RLVK.State.vertexCounter + 2] = tz;

    // Add current texcoord
    RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].texcoords[2*RLVK.State.vertexCounter] = RLVK.State.texcoordx;
    RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].texcoords[2*RLVK.State.vertexCounter + 1] = RLVK.State.texcoordy;

    // WARNING: By default rlVertexBuffer struct does not store normals

    // Add current color
    RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].colors[4*RLVK.State.vertexCounter] = RLVK.State.colorr;
    RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].colors[4*RLVK.State.vertexCounter + 1] = RLVK.State.colorg;
    RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].colors[4*RLVK.State.vertexCounter + 2] = RLVK.State.colorb;
    RLVK.currentBatch->vertexBuffer[RLVK.currentBatch->currentBuffer].colors[4*RLVK.State.vertexCounter + 3] = RLVK.State.colora;

    RLVK.State.vertexCounter++;
    RLVK.currentBatch->draws[RLVK.currentBatch->drawCounter - 1].vertexCount++;
}

// Define one vertex (position)
void rlVertex2f(float x, float y)
{
    rlVertex3f(x, y, RLVK.currentBatch->currentDepth);
}

// Finish vertex providing
void rlEnd(void)
{
    // NOTE: Depth increment is dependant on rlOrtho(): z-near and z-far values,
    // as well as depth buffer bit-depth (16bit or 24bit or 32bit)
    // Correct increment formula would be: depthInc = (zfar - znear)/pow(2, bits)
    RLVK.currentBatch->currentDepth += (1.0f/20000.0f);
}

// Get name string for pixel format
const char *rlGetPixelFormatName(unsigned int format)
{
    switch (format)
    {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: return "GRAYSCALE"; break;         // 8 bit per pixel (no alpha)
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: return "GRAY_ALPHA"; break;       // 8*2 bpp (2 channels)
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5: return "R5G6B5"; break;               // 16 bpp
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: return "R8G8B8"; break;               // 24 bpp
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: return "R5G5B5A1"; break;           // 16 bpp (1 bit alpha)
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: return "R4G4B4A4"; break;           // 16 bpp (4 bit alpha)
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: return "R8G8B8A8"; break;           // 32 bpp
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: return "R32"; break;                     // 32 bpp (1 channel - float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: return "R32G32B32"; break;         // 32*3 bpp (3 channels - float)
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: return "R32G32B32A32"; break;   // 32*4 bpp (4 channels - float)
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB: return "DXT1_RGB"; break;             // 4 bpp (no alpha)
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA: return "DXT1_RGBA"; break;           // 4 bpp (1 bit alpha)
        case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA: return "DXT3_RGBA"; break;           // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA: return "DXT5_RGBA"; break;           // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB: return "ETC1_RGB"; break;             // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB: return "ETC2_RGB"; break;             // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: return "ETC2_RGBA"; break;       // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB: return "PVRT_RGB"; break;             // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: return "PVRT_RGBA"; break;           // 4 bpp
        case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: return "ASTC_4x4_RGBA"; break;   // 8 bpp
        case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: return "ASTC_8x8_RGBA"; break;   // 2 bpp
        default: return "UNKNOWN"; break;
    }
}

// Read texture pixel data
void *rlReadTexturePixels(unsigned int id, int width, int height, int format)
{
    //TODO
}

unsigned int rlLoadTexture(const void *data, int width, int height, int format, int mipmapCount)
{
    
}

unsigned int rlLoadTextureCubemap(const void *data, int size, int format)
{

}

unsigned int rlLoadFramebuffer(int width, int height)
{
    
}

// Load depth texture/renderbuffer (to be attached to fbo)
// WARNING: OpenGL ES 2.0 requires GL_OES_depth_texture and WebGL requires WEBGL_depth_texture extensions
unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer)
{

}

// Attach color buffer texture to an fbo (unloads previous attachment)
// NOTE: Attach type: 0-Color, 1-Depth renderbuffer, 2-Depth texture
void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel)
{

}

// Verify render texture is complete
bool rlFramebufferComplete(unsigned int id)
{

}

// Unload texture from GPU memory
void rlUnloadTexture(unsigned int id)
{

}

// Unload framebuffer from GPU memory
// NOTE: All attached textures/cubemaps/renderbuffers are also deleted
void rlUnloadFramebuffer(unsigned int id)
{

}

// Update already loaded texture in GPU with new data
// NOTE: We don't know safely if internal texture format is the expected one...
void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data)
{

}

// Generate mipmap data for selected texture
// NOTE: Only supports GPU mipmap generation
void rlGenTextureMipmaps(unsigned int id, int width, int height, int format, int *mipmaps)
{

}

void rlSwapScreenBuffers(void)
{

}

void rlSetVSync(bool enable)
{
    RLVK.vSync = enable;

    if (RLVK.inited)
    {
        //TODO
    }
}

#endif