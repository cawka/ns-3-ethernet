## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('ethernet', ['network', 'csma'])
    module.source = [
        'model/ethernet-net-device.cc',
        'model/ethernet-channel.cc',
        'helpers/ethernet-helper.cc',
        ]
    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'ethernet'
    headers.source = [
        'model/ethernet-net-device.h',
        'model/ethernet-channel.h',
        'helpers/ethernet-helper.h',
        ]

    bld.ns3_python_bindings()
