#include "utils/log.h"
#include "utils/defer.hpp"
#include "platform/memory.h"
#include "skr_image_coder/skr_image_coder.h"
#include "image_coder_png.hpp"
#include "image_coder_jpeg.hpp"

void SkrImageCoderModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("image coder module loaded!");
}

void SkrImageCoderModule::on_unload()
{
    SKR_LOG_INFO("image coder module unloaded!");
}

IMPLEMENT_DYNAMIC_MODULE(SkrImageCoderModule, SkrImageCoder);

skr_image_coder_id skr_image_coder_create_image(EImageCoderFormat format)
{
    switch (format)
    {
    case IMAGE_CODER_FORMAT_PNG:
        return SkrNew<skr::PNGImageCoder>();
    case IMAGE_CODER_FORMAT_JPEG:
        return SkrNew<skr::JPEGImageCoder>();
    case IMAGE_CODER_FORMAT_GrayScaleJPEG:
    case IMAGE_CODER_FORMAT_BMP:
    case IMAGE_CODER_FORMAT_ICO:
    case IMAGE_CODER_FORMAT_EXR:
    case IMAGE_CODER_FORMAT_ICNS:
    case IMAGE_CODER_FORMAT_TGA:
    case IMAGE_CODER_FORMAT_HDR:
    case IMAGE_CODER_FORMAT_TIFF:
    default:
        SKR_UNIMPLEMENTED_FUNCTION()
        return nullptr;
    }
}

void skr_image_coder_free_image(skr_image_coder_id image)
{
    SkrDelete(image);
}

bool skr_image_coder_set_encoded(skr_image_coder_id image, const uint8_t* data, uint64_t size)
{
    return image->set_encoded(data, size);
}

bool skr_image_coder_move_encoded(skr_image_coder_id image, const uint8_t *data, uint64_t size)
{
    return image->move_encoded(data, size);
}

bool skr_image_coder_set_raw(skr_image_coder_id image, const uint8_t* data, uint64_t size, uint32_t width, uint32_t height, EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw)
{
    return image->set_raw(data, size, width, height, format, bit_depth, bytes_per_raw);
}

bool skr_image_coder_move_raw(skr_image_coder_id image, const uint8_t* data, uint64_t size, uint32_t width, uint32_t height, EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw)
{
    return image->move_raw(data, size, width, height, format, bit_depth, bytes_per_raw);
}
bool skr_image_coder_view_raw(skr_image_coder_id image, const uint8_t* data, 
    uint64_t size, uint32_t width, uint32_t height, 
    EImageCoderColorFormat format, uint32_t bit_depth, uint32_t bytes_per_raw)
{
    return image->view_raw(data, size, width, height, format, bit_depth, bytes_per_raw);
}

bool skr_image_coder_get_raw_data_view(skr_image_coder_id image, uint8_t** pData, uint64_t* pSize, EImageCoderColorFormat format, uint32_t bit_depth)
{
    auto _ = image->get_raw_data_view(format, bit_depth);
    *pData = (uint8_t*)_.data();
    *pSize = _.size();
    return _.size();
}

bool skr_image_coder_get_raw_data(skr_image_coder_id image, uint8_t* pData, uint64_t* pSize, EImageCoderColorFormat format, uint32_t bit_depth)
{
    return image->get_raw_data(pData, pSize, format, bit_depth);
}

bool skr_image_coder_get_encoded_data_view(skr_image_coder_id image, uint8_t** pData, uint64_t* pSize)
{
    auto _ = image->get_encoded_data_view();
    *pData = (uint8_t*)_.data();
    *pSize = _.size();
    return _.size();
}

bool skr_image_coder_get_encoded_data(skr_image_coder_id image, uint8_t* pData, uint64_t* pSize)
{
    return image->get_encoded_data(pData, pSize);
}

EImageCoderFormat skr_image_coder_get_image_format(skr_image_coder_id image)
{
    return image->get_image_format();
}

EImageCoderColorFormat skr_image_coder_get_color_format(skr_image_coder_id image)
{
    return image->get_color_format();
}

uint64_t skr_image_coder_get_raw_size(skr_image_coder_id image)
{
    return image->get_raw_size();
}

uint64_t skr_image_coder_get_encoded_size(skr_image_coder_id image)
{
    return image->get_encoded_size();
}

uint32_t skr_image_coder_get_width(skr_image_coder_id image)
{
    return image->get_width();
}

uint32_t skr_image_coder_get_height(skr_image_coder_id image)
{
    return image->get_height();
}

uint32_t skr_image_coder_get_bit_depth(skr_image_coder_id image)
{
    return image->get_bit_depth();
}

namespace
{
    static const uint8_t IMAGE_MAGIC_PNG[]  = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
	static const uint8_t IMAGE_MAGIC_JPEG[] = {0xFF, 0xD8, 0xFF};
	static const uint8_t IMAGE_MAGIC_BMP[]  = {0x42, 0x4D};
	static const uint8_t IMAGE_MAGIC_ICO[]  = {0x00, 0x00, 0x01, 0x00};
	static const uint8_t IMAGE_MAGIC_EXR[]  = {0x76, 0x2F, 0x31, 0x01};
	static const uint8_t IMAGE_MAGIC_ICNS[] = {0x69, 0x63, 0x6E, 0x73};

    template <int32_t MagicCount> 
    bool StartsWith(const uint8_t* Content, int64_t ContentSize, const uint8_t (&Magic)[MagicCount])
	{
		if (ContentSize < MagicCount)
		{
			return false;
		}

		for (int32_t I = 0; I < MagicCount; ++I)
		{
			if (Content[I] != Magic[I])
			{
				return false;
			}
		}

		return true;
	}
}

EImageCoderFormat skr_image_coder_detect_format(const uint8_t* encoded_data, uint64_t size)
{
    EImageCoderFormat format = IMAGE_CODER_FORMAT_INVALID;
    if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_PNG))
    {
        format = IMAGE_CODER_FORMAT_PNG;
    }
    else if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_JPEG))
    {
        format = IMAGE_CODER_FORMAT_JPEG; // @Todo: Should we detect grayscale vs non-grayscale?
    }
    else if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_BMP))
    {
        format = IMAGE_CODER_FORMAT_BMP;
    }
    else if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_ICO))
    {
        format = IMAGE_CODER_FORMAT_ICO;
    }
    else if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_EXR))
    {
        format = IMAGE_CODER_FORMAT_EXR;
    }
    else if (::StartsWith(encoded_data, size, ::IMAGE_MAGIC_ICNS))
    {
        format = IMAGE_CODER_FORMAT_ICNS;
    }
    return format;
}

#ifdef _WIN32
#include "skr_image_coder/extensions/win_dstorage_decompressor.h"
#include "tracy/Tracy.hpp"

HRESULT skr_image_coder_win_dstorage_decompressor(skr_win_dstorage_decompress_request_t* request, void* user_data)
{
    ZoneScopedN("DirectStoragePNGDecompressor");
    EImageCoderFormat format = skr_image_coder_detect_format((const uint8_t*)request->src_buffer, request->src_size);
    SKR_LOG_DEBUG("skr_image_coder_win_dstorage_decompressor: format=%d", format);
    auto coder = skr_image_coder_create_image(format);
    if (skr_image_coder_set_encoded(coder, (const uint8_t*)request->src_buffer, request->src_size))
    {
        SKR_DEFER({ ZoneScopedN("DirectStoragePNGDecompressorFree"); skr_image_coder_free_image(coder); });
        SKR_LOG_DEBUG("image coder: width = %d, height = %d, encoded_size = %d, raw_size = %d", 
            skr_image_coder_get_width(coder), skr_image_coder_get_height(coder), 
            skr_image_coder_get_encoded_size(coder),
            skr_image_coder_get_raw_size(coder));
        uint64_t actualSize = request->dst_size;
        const auto encoded_format = coder->get_color_format();
        const auto raw_format = (encoded_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : encoded_format;
        if (skr_image_coder_get_raw_data(coder, 
            (uint8_t*)request->dst_buffer, &actualSize, 
            raw_format, coder->get_bit_depth()))
        {
            /*
            {
                auto jcoder = skr_image_coder_create_image(IMAGE_CODER_FORMAT_JPEG);
                const auto jbit_depth = skr_image_coder_get_bit_depth(coder);
                const auto jwidth = skr_image_coder_get_width(coder);
                const auto jheight = skr_image_coder_get_height(coder);
                SKR_DEFER({ ZoneScopedN("DirectStorageJTestDecompressorFree"); skr_image_coder_free_image(jcoder); });
                bool jview = skr_image_coder_view_raw(jcoder, (const uint8_t*)request->dst_buffer, actualSize, 
                    jwidth, jheight,
                    raw_format, jbit_depth, 4 * jwidth * jbit_depth / 8);
                SKR_ASSERT(jview);
                
                uint8_t* encoded = nullptr;
                uint64_t encoded_size = 0;
                bool jencode = skr_image_coder_get_encoded_data_view(jcoder, &encoded, &encoded_size);
                SKR_ASSERT(jencode);
                
                bool jraw = skr_image_coder_get_raw_data(jcoder, 
                    (uint8_t*)request->dst_buffer, &actualSize, 
                    raw_format, coder->get_bit_depth());
                SKR_ASSERT(jraw);
            }
            */
            return 0L; // S_OK
        }
    }
    return 1L; // S_FALSE
}
#endif