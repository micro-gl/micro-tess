#define MICROGL_USE_EXTERNAL_MICRO_TESS
#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/samplers/texture.h>
#include <microgl/samplers/flat_color.h>

#include <micro-tess/dynamic_array.h>
#include <micro-tess/vec3.h>

template <typename number>
number * bi_cubic_1(){
    return new number[4 * 4 * 2] {
            // row 0
            1.0f,     0.0f,
            170.66f,  0.0f,
            341.333f, 0.0f,
            512.0f,   0.0f,

            // row 1
            1.0f,     170.66f,
            293.44f,  162.78f,
            746.68f,  144.65f,
            512.0f,   170.66f,

            // row 2
            1.0f,     341.33f,
            383.12f,  327.69f,
            1042.79f, 296.31f,
            512.0f,   341.33f,

            // row 3
            1.0f,     512.0f,
            170.66f,  512.0f,
            341.333f, 512.0f,
            512.0f,   512.0f,
    };
}

int main() {
    using number = float;
//    using number = Q<16>;
    using index = unsigned int;

    // microgl drawing setup START
    using Canvas24 = canvas<bitmap<coder::RGB888_PACKED_32>>;
    using Texture24 = sampling::texture<bitmap<coder::RGB888_ARRAY>, sampling::texture_filter::Bilinear>;
    sampling::flat_color<> color_grey{{222,222,222,255}};
    Canvas24 canvas(640, 480);
    auto img_2 = Resources::loadImageFromCompressedPath("images/uv_512.png");
    Texture24 tex_uv{new bitmap<coder::RGB888_ARRAY>(img_2.data, img_2.width, img_2.height)};
    // microgl drawing setup END

    constexpr int samples = 20;
    constexpr bool debug = true;

    auto draw_bezier_patch = [&](number* mesh) {
        // Algorithm START
        // output vertices attributes
        dynamic_array<number> output_attrib;
        // output indices
        dynamic_array<unsigned int> output_indices;
        // output triangles type
        microtess::triangles::indices output_indices_type;
        // define algorithm
        using tess= microtess::bezier_patch_tesselator<
                number, number,
                decltype(output_attrib),
                decltype(output_indices)>;
        using vertex=microtess::vec2<number>;
        // compute algorithm
        auto window_size = tess::compute<microtess::patch_type::BI_CUBIC>(
                mesh, 2,
                samples, samples,
                true, true,
                output_attrib,output_indices, output_indices_type,
                0, 0, 1, 1);
        // Algorithm END

        canvas.clear({255, 255, 255, 255});
        int I_X = 0, I_Y=1, I_U=2, I_V=3;
        // walk on pieces of triangles with inverting because it is a TRIANGLES_STRIP
        bool even = true;
        for (index ix = 0; ix < output_indices.size()-2; ++ix, even=!even) { // we alternate order inorder to preserve CCW or CW,
            index first_index   = (even ? output_indices[ix + 0] : output_indices[ix + 2]) * window_size;
            index second_index  = (even ? output_indices[ix + 1] : output_indices[ix + 1]) * window_size;
            index third_index   = (even ? output_indices[ix + 2] : output_indices[ix + 0]) * window_size;

            vertex p1=vertex{output_attrib[first_index + I_X], output_attrib[first_index + I_Y]};
            vertex p2=vertex{output_attrib[second_index + I_X], output_attrib[second_index + I_Y]};
            vertex p3=vertex{output_attrib[third_index + I_X], output_attrib[third_index + I_Y]};

            canvas.drawTriangle<>(
                    tex_uv,
                    p1.x, p1.y, output_attrib[first_index  + I_U], output_attrib[first_index  + I_V],
                    p2.x, p2.y, output_attrib[second_index + I_U], output_attrib[second_index + I_V],
                    p3.x, p3.y, output_attrib[third_index  + I_U], output_attrib[third_index  + I_V]);
            if(debug)
                canvas.drawTriangleWireframe<number>(color_t{0,0,0,255},
                                               {output_attrib[first_index + I_X], output_attrib[first_index + I_Y]},
                                               {output_attrib[second_index + I_X], output_attrib[second_index + I_Y]},
                                               {output_attrib[third_index + I_X], output_attrib[third_index + I_Y]});
        }
    };

    auto render = [&](void*, void*, void*) -> void {
        draw_bezier_patch(bi_cubic_1<number>());
    };

    example_run(&canvas, render);
}
