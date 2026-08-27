#!/usr/bin/env python3
import pathlib, tempfile, numpy as np, sys
ROOT=pathlib.Path(__file__).resolve().parents[3];sys.path.insert(0,str(ROOT/'scripts/super_livo/evaluation'))
from fast_livo2_debug_to_tum import convert
with tempfile.TemporaryDirectory() as td:
    td=pathlib.Path(td);src=td/'in.txt';dst=td/'out.tum'
    src.write_text('0 0 0 0 1 2 3 0 0 0 0 0 0 0 0 0 1 0 0 10\n1 0 0 90 4 5 6 0 0 0 0 0 0 0 0 0 1 0 0 11\n')
    convert(src,dst,100.0);d=np.loadtxt(dst)
    assert np.allclose(d[0],[100,1,2,3,0,0,0,1])
    assert np.allclose(d[1],[101,4,5,6,0,0,2**-.5,2**-.5])
print('FAST-LIVO2 DEBUG TO TUM: ALL PASS')
