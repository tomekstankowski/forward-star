from distutils.core import setup, Extension

setup(name="simplegraphs", version="1.0",
      ext_modules=[Extension("simplegraphs", ["forward_star.c"])])