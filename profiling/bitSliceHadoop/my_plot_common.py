#!/usr/bin/python

# -*- coding: utf-8 -*-
"""
Created on Fri Jan 08 09:57:53 2016

@author: Josiah
"""


import matplotlib.cm as cmx
import matplotlib.colors as colors
import numpy as np

def get_cmap(N):
    '''Returns a function that maps each index in 0, 1, ... N-1 to a distinct
    RGB color.'''
    color_norm  = colors.Normalize(vmin=0, vmax=N-1)
    scalar_map = cmx.ScalarMappable(norm=color_norm, cmap='hsv')
    def map_index_to_rgb_color(index):
        return scalar_map.to_rgba(index)
    return map_index_to_rgb_color

def get_mean_with_err(data):
	mean = np.mean(data,axis=0)
	mx = np.max(data,axis=0)
	mn = np.min(data,axis=0)

	yerr = [mx - mean, mean - mn]
	return (mean, yerr)

