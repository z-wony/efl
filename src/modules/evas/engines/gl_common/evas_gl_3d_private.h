#ifndef EVAS_GL_3D_PRIVATE_H
#define EVAS_GL_3D_PRIVATE_H

#include "evas_gl_common.h"

typedef struct _E3D_Program   E3D_Program;
typedef struct _E3D_Draw_Data E3D_Draw_Data;
typedef unsigned long         E3D_Shader_Flag;

// NOTE: order here should be equal with flag names in file evas_gl_3d_shader.c
#define E3D_SHADER_FLAG_NORMALIZE_NORMALS       (1 << 0)
#define E3D_SHADER_FLAG_VERTEX_POSITION         (1 << 1)
#define E3D_SHADER_FLAG_VERTEX_POSITION_BLEND   (1 << 2)
#define E3D_SHADER_FLAG_VERTEX_NORMAL           (1 << 3)
#define E3D_SHADER_FLAG_VERTEX_NORMAL_BLEND     (1 << 4)
#define E3D_SHADER_FLAG_VERTEX_TANGENT          (1 << 5)
#define E3D_SHADER_FLAG_VERTEX_TANGENT_BLEND    (1 << 6)
#define E3D_SHADER_FLAG_VERTEX_COLOR            (1 << 7)
#define E3D_SHADER_FLAG_VERTEX_COLOR_BLEND      (1 << 8)
#define E3D_SHADER_FLAG_VERTEX_TEXCOORD         (1 << 9)
#define E3D_SHADER_FLAG_VERTEX_TEXCOORD_BLEND   (1 << 10)
#define E3D_SHADER_FLAG_LIGHT_DIRECTIONAL       (1 << 11)
#define E3D_SHADER_FLAG_LIGHT_SPOT              (1 << 12)
#define E3D_SHADER_FLAG_LIGHT_ATTENUATION       (1 << 13)
#define E3D_SHADER_FLAG_AMBIENT                 (1 << 14)
#define E3D_SHADER_FLAG_DIFFUSE                 (1 << 15)
#define E3D_SHADER_FLAG_SPECULAR                (1 << 16)
#define E3D_SHADER_FLAG_EMISSION                (1 << 17)
#define E3D_SHADER_FLAG_DIFFUSE_TEXTURE         (1 << 18)
#define E3D_SHADER_FLAG_AMBIENT_TEXTURE         (1 << 19)
#define E3D_SHADER_FLAG_SPECULAR_TEXTURE        (1 << 20)
#define E3D_SHADER_FLAG_EMISSION_TEXTURE        (1 << 21)
#define E3D_SHADER_FLAG_NORMAL_TEXTURE          (1 << 22)
#define E3D_SHADER_FLAG_DIFFUSE_TEXTURE_BLEND   (1 << 23)
#define E3D_SHADER_FLAG_AMBIENT_TEXTURE_BLEND   (1 << 24)
#define E3D_SHADER_FLAG_SPECULAR_TEXTURE_BLEND  (1 << 25)
#define E3D_SHADER_FLAG_EMISSION_TEXTURE_BLEND  (1 << 26)
#define E3D_SHADER_FLAG_NORMAL_TEXTURE_BLEND    (1 << 27)
#define E3D_SHADER_FLAG_FOG_ENABLED             (1 << 28)
#define E3D_SHADER_FLAG_ALPHA_TEST_ENABLED      (1 << 29)
#define E3D_SHADER_FLAG_SHADOWED                (1 << 30)
#define E3D_SHADER_FLAG_COUNT                    31

static inline Eina_Bool
_flags_need_tex_coord(E3D_Shader_Flag flags)
{
   return (flags & E3D_SHADER_FLAG_AMBIENT_TEXTURE) ||
          (flags & E3D_SHADER_FLAG_DIFFUSE_TEXTURE) ||
          (flags & E3D_SHADER_FLAG_SPECULAR_TEXTURE) ||
          (flags & E3D_SHADER_FLAG_EMISSION_TEXTURE) ||
          (flags & E3D_SHADER_FLAG_NORMAL_TEXTURE);
}

struct _E3D_Draw_Data
{
   E3D_Shader_Flag      flags;
   Evas_Canvas3D_Shade_Mode   mode;

   Evas_Mat4   matrix_mvp;
   Evas_Mat4   matrix_mv;
   Evas_Mat3   matrix_normal;
   Evas_Mat4   matrix_light;

   struct {
        Evas_Canvas3D_Vertex_Buffer vertex0;
        Evas_Canvas3D_Vertex_Buffer vertex1;
        Evas_Real             weight;
   } vertices[EVAS_CANVAS3D_VERTEX_ATTRIB_COUNT];

   Evas_Canvas3D_Vertex_Assembly assembly;
   int                     vertex_count;
   int                     index_count;
   Evas_Canvas3D_Index_Format    index_format;
   const void             *indices;

   GLint       texture_count;

   struct {
        Evas_Color            color;
        GLint                 sampler0;
        GLint                 sampler1;
        E3D_Texture          *tex0;
        E3D_Texture          *tex1;
        Evas_Real             texture_weight;
   } materials[EVAS_CANVAS3D_MATERIAL_ATTRIB_COUNT];

   Evas_Real shininess;

   GLint                   smap_sampler;
   Evas_Canvas3D_Blend_Func      blend_sfactor;
   Evas_Canvas3D_Blend_Func      blend_dfactor;
   Eina_Bool               blending : 1;

   Evas_Canvas3D_Comparison      alpha_comparison;
   Evas_Real               alpha_ref_value;
   Eina_Bool               alpha_test_enabled :1;

   struct {
        Evas_Vec4   position;
        Evas_Vec3   spot_dir;
        Evas_Real   spot_exp;
        Evas_Real   spot_cutoff_cos;
        Evas_Vec3   atten;
        Evas_Color  ambient;
        Evas_Color  diffuse;
        Evas_Color  specular;
   } light;
   Evas_Color fog_color;
#ifndef GL_GLES
   double color_pick_key;
#else
   Evas_Color color_pick_key;
#endif
   /*Sets of the quality of shadow rendering*/
   Evas_Real               pcf_step;
   Evas_Real               pcf_size;
#ifdef GL_GLES
   Evas_Real               constant_bias;
#endif
};

struct _E3D_Texture
{
   /*Offset for atlasses*/
   int               x, y;
   int               w, h;

   Evas_GL_Image     *surface;
   /*Tranformation matrix, use it for adjusting texture unit coordinates*/
   Evas_Mat3         trans;

   GLuint            tex;

   Eina_Bool         wrap_dirty;
   GLenum            wrap_s;
   GLenum            wrap_t;

   Eina_Bool         filter_dirty;
   GLenum            filter_min;
   GLenum            filter_mag;
   /*Use atlas for generation texture unit, @EINA_TRUE by default*/
   Eina_Bool         atlas_enable;
};

struct _E3D_Drawable
{
   int      w, h;
   int      alpha;
   GLenum   format;
   GLenum   depth_format;
   GLenum   stencil_format;
   GLuint   tex;
   GLuint   fbo;
   GLuint   depth_stencil_buf;
   GLuint   depth_buf;
   GLuint   stencil_buf;
   GLuint   texDepth;
   GLuint texcolorpick;
   GLuint color_pick_fb_id;
#ifdef GL_GLES
   GLuint   shadow_fbo;
   GLuint   depth_render_buf;
#endif
};

/* Texture internal functions. */
void                 e3d_texture_param_update(E3D_Texture *texture);

/* Program */
E3D_Program         *e3d_program_new(Evas_Canvas3D_Shade_Mode mode, E3D_Shader_Flag flags);
void                 e3d_program_free(E3D_Program *program);
GLuint               e3d_program_id_get(const E3D_Program *program);
Evas_Canvas3D_Shade_Mode   e3d_program_shade_mode_get(const E3D_Program *program);
E3D_Shader_Flag      e3d_program_shader_flags_get(const E3D_Program *program);
void                 e3d_program_uniform_upload(E3D_Program *program, const E3D_Draw_Data *data);

/* Renderer */
void                 e3d_renderer_target_set(E3D_Renderer *renderer, E3D_Drawable *target);
void                 e3d_renderer_viewport_set(E3D_Renderer *renderer, int x, int y, int w, int h);
void                 e3d_renderer_clear(E3D_Renderer *renderer, const Evas_Color *color);
void                 e3d_renderer_draw(E3D_Renderer *renderer, E3D_Draw_Data *data);
void                 e3d_renderer_flush(E3D_Renderer *renderer);

#endif /* EVAS_GL_3D_PRIVATE_H */
