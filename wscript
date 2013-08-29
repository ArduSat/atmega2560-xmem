#!/usr/bin/env python
# encoding: utf-8

import os

APPNAME = 'atmega2560-xmem'
VERSION = '1.0'

def options(ctx):
  ctx.add_option('--toolchain', action='store', default='avr', help='Set toolchain prefix')
  ctx.add_option('--mcu', default='atmega2560', help='Set CPU type')
  ctx.add_option('--fcpu', default='8000000UL', help='Set CPU frequency')

  gr = ctx.add_option_group('atmega2560-xmem options')
  gr.add_option('--with-xmem-config-path', metavar="CONFIG", default='module_config', help='Set the path to the configuration file.')

def configure(ctx):
  ctx.env.CC = ctx.options.toolchain + "-gcc"
  ctx.env.AR = ctx.options.toolchain + "-ar"

  mcu = ctx.env.MCU = ctx.options.mcu
  fcpu = ctx.env.FCPU = ctx.options.fcpu


  ctx.env.append_unique('CFLAGS', [
    '-Wall', '-Wextra', '-Wno-unused-parameter',
    '-Wno-missing-field-initializers', '-Os', '-std=gnu99', '-fshort-enums',
    '-funsigned-char', '-funsigned-bitfields', '-fdata-sections',
    '-ffunction-sections', '-mmcu='+mcu,
    '-DF_CPU=' + fcpu
  ])

  ctx.load('gcc')
  ctx.find_program(ctx.options.toolchain + '-size', var='SIZE')

  # Setup CFLAGS
  ctx.env.prepend_value('CFLAGS', ['-Os','-Wall', '-g', '-std=gnu99'])

  if ctx.options.with_xmem_config_path:
    ctx.options.with_xmem_config_path = os.path.abspath(ctx.options.with_xmem_config_path)

def build(ctx):
  install_path = False

#  ctx.install_files('${PREFIX}', ctx.path.ant_glob('**/*.h'), relative_trick=True)

  ctx.stlib(source=ctx.path.ant_glob('src/**/*.c'),
            target='atmega2560-xmem',
            includes = ['include', ctx.options.with_xmem_config_path],
            export_includes = ['include'],
            cflags = ctx.env.CFLAGS + ['-Wno-cast-align'],
            install_path = install_path)
