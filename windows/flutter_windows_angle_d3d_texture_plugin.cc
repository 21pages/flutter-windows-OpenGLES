#include "flutter_windows_angle_d3d_texture_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <flutter/texture_registrar.h>

namespace flutter_windows_angle_d3d_texture {

constexpr auto kTextureWidth = 1920;
constexpr auto kTextureHeight = 1080;

GLuint CompileShader(GLenum type, const std::string& source) {
  auto shader = glCreateShader(type);
  const char* s[1] = {source.c_str()};
  glShaderSource(shader, 1, s, NULL);
  glCompileShader(shader);
  auto result = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
  if (result == 0) {
    auto size = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
    std::vector<GLchar> text(size);
    glGetShaderInfoLog(shader, static_cast<GLsizei>(text.size()), NULL,
                       text.data());
    auto error = std::string("Shader Compilation Failed:\n");
    error += text.data();
    std::cout << error << std::endl;
  }
  return shader;
}

GLuint CompileProgram(const std::string& vertex_shader_source,
                      const std::string& fragment_shader_source) {
  auto program = glCreateProgram();
  if (program == 0) {
    std::cout << "Program Creation Failed." << std::endl;
  }
  auto vs = CompileShader(GL_VERTEX_SHADER, vertex_shader_source);
  auto fs = CompileShader(GL_FRAGMENT_SHADER, fragment_shader_source);
  if (vs == 0 || fs == 0) {
    glDeleteShader(fs);
    glDeleteShader(vs);
    glDeleteProgram(program);
    return 0;
  }
  glAttachShader(program, vs);
  glDeleteShader(vs);
  glAttachShader(program, fs);
  glDeleteShader(fs);
  glLinkProgram(program);
  auto status = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == 0) {
    auto size = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);
    std::vector<GLchar> text(size);
    glGetProgramInfoLog(program, static_cast<GLsizei>(text.size()), NULL,
                        text.data());
    auto error = std::string("Program Link Failed:\n");
    error += text.data();
    std::cout << error << std::endl;
  }
  return program;
}

void FlutterWindowsAngleD3dTexturePlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows* registrar) {
  auto plugin = std::make_unique<FlutterWindowsAngleD3dTexturePlugin>(
      registrar,
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(),
          "flutter-windows-ANGLE-OpenGL-Direct3D-Interop",
          &flutter::StandardMethodCodec::GetInstance()),
      registrar->texture_registrar());
  plugin->channel()->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto& call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });
  registrar->AddPlugin(std::move(plugin));
}

FlutterWindowsAngleD3dTexturePlugin::FlutterWindowsAngleD3dTexturePlugin(
    flutter::PluginRegistrarWindows* registrar,
    std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel,
    flutter::TextureRegistrar* texture_registrar)
    : registrar_(registrar),
      channel_(std::move(channel)),
      texture_registrar_(texture_registrar) {}

FlutterWindowsAngleD3dTexturePlugin::~FlutterWindowsAngleD3dTexturePlugin() {}

void FlutterWindowsAngleD3dTexturePlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("render") == 0) {
    auto window = registrar_->GetView()->GetNativeWindow();
    auto adapter = registrar_->GetView()->GetGraphicsAdapter();
    surface_manager_ = std::make_unique<ANGLESurfaceManager>(
        window, kTextureWidth, kTextureHeight, adapter);
    texture_ = std::make_unique<FlutterDesktopGpuSurfaceDescriptor>();
    texture_->struct_size = sizeof(FlutterDesktopGpuSurfaceDescriptor);
    texture_->handle = surface_manager_->shared_handle();
    texture_->width = texture_->visible_width = kTextureWidth;
    texture_->height = texture_->visible_height = kTextureHeight;
    texture_->release_context = nullptr;
    texture_->release_callback = [](void* release_context) {};
    texture_->format = kFlutterDesktopPixelFormatBGRA8888;
    texture_variant_ =
        std::make_unique<flutter::TextureVariant>(flutter::GpuSurfaceTexture(
            kFlutterDesktopGpuSurfaceTypeDxgiSharedHandle,
            [&](auto, auto) { return texture_.get(); }));

    eglMakeCurrent(surface_manager_->display(), surface_manager_->surface(),
                   surface_manager_->surface(), surface_manager_->context());
    constexpr char kVertexShader[] = R"(attribute vec4 vPosition;
    void main()
    {
        gl_Position = vPosition;
    })";
    constexpr char kFragmentShader[] = R"(precision mediump float;
    void main()
    {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    })";
    auto program = CompileProgram(kVertexShader, kFragmentShader);
    glEnableVertexAttribArray(0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GLfloat vertices[] = {
        0.0f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f,
    };
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, kTextureWidth, kTextureHeight);
    glUseProgram(program);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
    surface_manager_->SwapBuffers();
    texture_id_ = texture_registrar_->RegisterTexture(texture_variant_.get());
    texture_registrar_->MarkTextureFrameAvailable(texture_id_);
    result->Success(flutter::EncodableValue(texture_id_));
  } else if (method_call.method_name().compare("destroy") == 0) {
    surface_manager_.reset();
    texture_registrar_->UnregisterTexture(texture_id_);
    result->Success(flutter::EncodableValue(std::monostate{}));
  } else {
    result->NotImplemented();
  }
}

}  // namespace flutter_windows_angle_d3d_texture
