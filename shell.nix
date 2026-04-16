{ pkgs ? import <nixpkgs> {
    config.allowUnfree = true; # CUDA проприетарна, это разрешение обязательно
  }
}:

let
  # Список необходимых библиотек для LD_LIBRARY_PATH
  gpuLibs = with pkgs; [
    metis
    cudaPackages.cudatoolkit
    linuxPackages.nvidia_x11 # Важно для работы с драйвером напрямую
    libGL
    libGLU
    freeglut
    glew
    glfw
    stdenv.cc.cc.lib # Стандартные библиотеки C++
  ];
in
pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    gnumake
    cmake

    gdb

    metis

    # CUDA
    cudaPackages.cudatoolkit

    # OpenGL & Windowing
    glfw
    glew
    libGL
    freeglut
    libGLU
    glm

    # X11
    xorg.libX11
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
  ];

  shellHook = ''
    # Путь к заголовочным файлам CUDA (для nvcc и компиляторов)
    export CUDA_PATH=${pkgs.cudaPackages.cudatoolkit}

    # Чтобы компилятор нашел cuda_runtime.h и cuda_gl_interop.h
    export CPATH="${pkgs.cudaPackages.cudatoolkit}/include:$CPATH"
    export LIBRARY_PATH="${pkgs.cudaPackages.cudatoolkit}/lib:$LIBRARY_PATH"

    # Динамическая линковка во время выполнения
    export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath gpuLibs}:$LD_LIBRARY_PATH"

    echo "CUDA + OpenGL Environment Loaded!"
    echo "CUDA_PATH: $CUDA_PATH"
  '';
}
