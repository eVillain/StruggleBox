#include "TextureLoader.h"

#include "Allocator.h"
#include "ArenaOperators.h"
#include "Texture2D.h"
#include "ThreadPool.h"
#include "Log.h"
#include "GFXDefines.h"
#include "GLUtils.h"
#include "PNG/png.h"
#include "PNG/pngstruct.h"

TextureLoader::TextureLoader(Allocator& allocator, ThreadPool& threadPool)
    : m_allocator(allocator)
    , m_threadPool(threadPool)
{
}

png_byte* loadPNGImageData(const std::string& fileName, Allocator& allocator, png_uint_32& width, png_uint_32& height, int& color_type)
{
#ifdef _WIN32
    FILE* fp = NULL;
    fopen_s(&fp, fileName.c_str(), "rb");
#else
    FILE* fp = fopen(fileName.c_str(), "rb");
#endif
    if (fp == NULL)
    {
        Log::Error("[TextureLoader] could not open %s", fileName.c_str());
        return nullptr;
    }

    // Read and check the header
    png_byte header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8))
    {
        Log::Error("[TextureLoader] header signature check failed, %s is not a PNG.", fileName.c_str());
        fclose(fp);
        return nullptr;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        Log::Error("[TextureLoader] png_create_read_struct returned 0.");
        fclose(fp);
        return nullptr;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        Log::Error("[TextureLoader] error: png_create_info_struct returned 0.");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return nullptr;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        Log::Error("[TextureLoader] error: png_create_info_struct returned 0.");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        fclose(fp);
        return nullptr;
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        Log::Error("[TextureLoader] error in png data");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return nullptr;
    }

    // init png reading
    png_init_io(png_ptr, fp);
    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);
    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);
    // Row size in bytes.
    unsigned long rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes - 1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    size_t image_bytes = rowbytes * height * sizeof(png_byte) + 15;
    png_byte* image_data = (png_byte*)allocator.allocate(image_bytes);
    if (image_data == NULL)
    {
        Log::Error("[TextureLoader] could not allocate memory for PNG image data (%i bytes)", image_bytes);
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return nullptr;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_bytep* row_pointers = (png_bytep*)allocator.allocate(height * sizeof(png_bytep));
    if (row_pointers == NULL)
    {
        Log::Error("[TextureLoader] could not allocate memory for PNG row pointers (%i bytes)", height * sizeof(png_bytep));
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        //free(image_data);
        allocator.deallocate(image_data);
        fclose(fp);
        return nullptr;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    for (unsigned int i = 0; i < height; i++)
    {
        row_pointers[height - 1 - i] = image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);

    allocator.deallocate(row_pointers);

    return image_data;
}

Texture2D* TextureLoader::loadFromFile(
    const std::string& fileName,
    GLint wrap /*= GL_REPEAT*/,
    GLint minF /*= GL_NEAREST*/,
    GLint magF /*= GL_NEAREST*/)
{
    int color_type;
    png_uint_32 temp_width, temp_height;

    png_byte* image_data = loadPNGImageData(fileName, m_allocator, temp_width, temp_height, color_type);
    if (!image_data)
    {
        Log::Error("[TextureLoader] could not load %s", fileName.c_str());
        return nullptr;
    }
    uint32_t formatGL = pngColorFormatToGL(color_type);
    GLuint textureIDGL = GLUtils::createTexture(temp_width, temp_height, 0, formatGL, GL_UNSIGNED_BYTE, wrap, minF, magF, false, image_data);

    m_allocator.deallocate(image_data);

    Texture2D* texture = CUSTOM_NEW(Texture2D, m_allocator)(textureIDGL, temp_width, temp_height, formatGL, GL_UNSIGNED_BYTE, wrap, minF, magF);

	return texture;
}


void ReadDataFromInputStream(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead)
{
    if (png_ptr->io_ptr == NULL)
    {
        Log::Error("Error reading png data from input stream");
        return;   // add custom error handling here
    }
    // -> replace with your own data source interface
    const char* inputData = (const char*)png_ptr->io_ptr;

    memcpy(outBytes, inputData, (size_t)byteCountToRead);
}

int ReadChunkCustom(png_structp ptr, png_unknown_chunkp chunk)
{
    static int unknown_numb = 0;

    Log::Info("Reading custom chunk [%s] @%p (%zu octets)", chunk->name, chunk->data, chunk->size);

    unknown_numb++;
    return unknown_numb;
}

Texture2D* TextureLoader::loadFromPNGData(const char* data)
{
    png_byte header[8];

    // Read and check the header
    memcpy(header, data, 8);

    if (png_sig_cmp(header, 0, 8))
    {
        Log::Debug("[Texture2D] error: data is not a PNG.\n");
        return nullptr;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) 
    {
        Log::Debug("[Texture2D] error: png_create_read_struct returned 0.");
        return nullptr;
    }


    //    // IGNORE UNKNOWN CHUNKS
    //    static /* const */ png_byte chunks_to_ignore[] = {
    //        01, 00, 00, 00, '\0',
    //        99,  72,  82,  77, '\0',  /* cHRM */
    //        104,  73,  83,  84, '\0',  /* hIST */
    //        105,  67,  67,  80, '\0',  /* iCCP */
    //        105,  84,  88, 116, '\0',  /* iTXt */
    //        111,  70,  70, 115, '\0',  /* oFFs */
    //        112,  67,  65,  76, '\0',  /* pCAL */
    //        112,  72,  89, 115, '\0',  /* pHYs */
    //        115,  66,  73,  84, '\0',  /* sBIT */
    //        115,  67,  65,  76, '\0',  /* sCAL */
    //        115,  80,  76,  84, '\0',  /* sPLT */
    //        115,  84,  69,  82, '\0',  /* sTER */
    //        116,  69,  88, 116, '\0',  /* tEXt */
    //        116,  73,  77,  69, '\0',  /* tIME */
    //        122,  84,  88, 116, '\0'   /* zTXt */
    //    };
    //    png_set_keep_unknown_chunks(png_ptr, 1 /* PNG_HANDLE_CHUNK_NEVER */,
    //                                chunks_to_ignore, sizeof(chunks_to_ignore)/5);

    //    png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_NEVER, NULL, 0);

        // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        Log::Error("[Texture2D] error: png_create_info_struct returned 0.");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        return nullptr;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        Log::Error("[Texture2D] error: png_create_info_struct returned 0.");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        return nullptr;
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) 
    {
        Log::Error("[Texture2D] error from libpng");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return nullptr;
    }
    // set data loading func
    png_set_read_fn(png_ptr, &data, ReadDataFromInputStream);

    // init png reading
//    png_init_io(png_ptr, data);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);


#if defined(PNG_HANDLE_AS_UNKNOWN_SUPPORTED)
    //    
    //	// Setting unknown-chunk callback
    //	png_set_read_user_chunk_fn(png_ptr, NULL, (png_user_chunk_ptr)ReadChunkCustom);
    //	
    //    /* Ignore unused chunks */
    //    png_set_keep_unknown_chunks(png_ptr, 3, NULL, 0);
    //    printf("ALL GOOD!\n");
#endif
#if defined(PNG_BENIGN_ERRORS_SUPPORTED)
    printf("Errors benign\n");
#endif
#if defined(PNG_BENIGN_READ_ERRORS_SUPPORTED)
    printf("Errors read benign\n");
#endif
    png_set_benign_errors(png_ptr, 1);
    printf("fail here?\n");
    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);
    printf("failed already?\n");

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    int width = temp_width;
    int height = temp_height;
    int bpp = bit_depth;
    uint32_t formatGL = pngColorFormatToGL(color_type);

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);
    // Row size in bytes.
    unsigned long rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes - 1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte* image_data;
    image_data = (unsigned char*)malloc(rowbytes * temp_height * sizeof(png_byte) + 15);
    if (image_data == NULL)
    {
        Log::Debug("error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return nullptr;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_bytep* row_pointers = (unsigned char**)malloc(temp_height * sizeof(png_bytep));
    if (row_pointers == NULL)
    {
        Log::Debug("error: could not allocate memory for PNG row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        return nullptr;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    for (unsigned int i = 0; i < temp_height; i++)
    {
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    // Generate the OpenGL Texture2D object
    GLuint textureIDGL = GLUtils::createTexture(temp_width, temp_height, 0, formatGL, GL_UNSIGNED_BYTE, GL_REPEAT, GL_NEAREST, GL_NEAREST, false, image_data);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);

    return nullptr;
}

void TextureLoader::asyncLoadFromFile(const std::string& fileName, const std::function<void(Texture2D*)>& callback, GLint wrap, GLint minF, GLint magF)
{
    TextureLoadPackage* package = CUSTOM_NEW(TextureLoadPackage, m_allocator){ 
        fileName,
        callback,
        wrap, minF, magF,
        0, 0, 0,
        nullptr,
        false
    };

    std::lock_guard<std::mutex> lock(m_mutex);
    m_packages.push_back(package);

    Log::Debug("TextureLoader::asyncLoadFromFile adding package at %p (%s)", package, package->fileName.c_str());

    m_threadPool.addJob(0, &TextureLoader::LoadPackageInThread, package, &m_allocator, &m_mutex);
}

void TextureLoader::processQueue()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (TextureLoadPackage* packageP : m_packages)
    {
        TextureLoadPackage& package = *packageP;
        if (!package.finished)
        {
            continue;
        }
        if (!package.image_data)
        {
            Log::Error("TextureLoader::processQueue failed to load package at %p (%s)", packageP, package.fileName.c_str());
            continue;
        }

        Log::Debug("TextureLoader::processQueue loaded package at %p (%s)", packageP, package.fileName.c_str());

        GLuint textureIDGL = GLUtils::createTexture(package.width, package.height, 0, package.formatGL, GL_UNSIGNED_BYTE, package.wrap, package.minF, package.magF, false, package.image_data);
        Texture2D* texture = CUSTOM_NEW(Texture2D, m_allocator)(textureIDGL, package.width, package.height, package.formatGL, GL_UNSIGNED_BYTE, package.wrap, package.minF, package.magF);

        package.callback(texture);

        m_allocator.deallocate(package.image_data);
        m_allocator.deallocate(packageP);
    }

    m_packages.erase(
        std::remove_if(
            m_packages.begin(),
            m_packages.end(),
            [](TextureLoadPackage* package) { return package->finished; }),
        m_packages.end());
}

void TextureLoader::LoadPackageInThread(TextureLoadPackage* package, Allocator* allocator, std::mutex* mutex)
{

    Log::Debug("TextureLoader::LoadPackageInThread starting package at %p (%s)", package, package->fileName.c_str());

    int color_type;
    package->image_data = loadPNGImageData(package->fileName, *allocator, package->width, package->height, color_type);
    package->formatGL = pngColorFormatToGL(color_type);

    std::lock_guard<std::mutex> lock(*mutex);
    package->finished = true;

    Log::Debug("TextureLoader::LoadPackageInThread finished package at %p (%s)", package, package->fileName.c_str());
}

uint32_t TextureLoader::pngColorFormatToGL(const int pngColorFormat)
{
    if (pngColorFormat == PNG_COLOR_TYPE_RGB)
    {
        return GL_RGB;
    }
    else if (pngColorFormat == PNG_COLOR_TYPE_RGB_ALPHA)
    {
        return GL_RGBA;
    }
    else if (pngColorFormat == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        return GL_RED;
    }
    else
    {
        Log::Error("[TextureLoader] Unknown Color format %i!", pngColorFormat);
    }
    return GL_RGBA;
}
