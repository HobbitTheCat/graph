{ pkgs ? import <nixpkgs> {
    config.allowUnfree = true;
  }
}:

let
  gpuLibs = with pkgs; [
    metis
    cudaPackages.cudatoolkit
    linuxPackages.nvidia_x11
    libGL
    libGLU
    freeglut
    glew
    glfw
    stdenv.cc.cc.lib
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
    export CUDA_PATH=${pkgs.cudaPackages.cudatoolkit}

    export CPATH="${pkgs.cudaPackages.cudatoolkit}/include:$CPATH"
    export LIBRARY_PATH="${pkgs.cudaPackages.cudatoolkit}/lib:$LIBRARY_PATH"

    export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath gpuLibs}:$LD_LIBRARY_PATH"

    echo "CUDA + OpenGL Environment Loaded!"
    echo "CUDA_PATH: $CUDA_PATH"
  '';
}
