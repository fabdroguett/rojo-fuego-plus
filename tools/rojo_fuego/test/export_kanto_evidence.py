"""Export unfiltered 240x160 screenshots and lossless contact sheets from actual PPMs."""
from pathlib import Path
from ppm2png import convert

ROOT=Path(__file__).resolve().parents[3]
OUT=ROOT/'build_artifacts/ui/kanto-art'


def read(path):
    data=path.read_bytes().split(b'\n',3)
    assert data[:1]==[b'P6']
    w,h=map(int,data[1].split())
    return w,h,data[3]


def sheet(paths,name,cols):
    images=[read(p) for p in paths]
    assert all((w,h)==(240,160) for w,h,_ in images)
    width,height=240*cols,160*((len(paths)+cols-1)//cols)
    result=bytearray(width*height*3)
    for i,(_,_,pixels) in enumerate(images):
        for y in range(160):
            offset=((i//cols*160+y)*width+(i%cols)*240)*3
            result[offset:offset+720]=pixels[y*720:(y+1)*720]
    ppm=OUT/(name+'.ppm')
    ppm.write_bytes(f'P6\n{width} {height}\n255\n'.encode()+result)
    convert(str(ppm),str(OUT/(name+'.png')),1)


def main():
    for ppm in OUT.rglob('*.ppm'):
        w,h,_=read(ppm)
        if (w,h)==(240,160): convert(str(ppm),str(ppm.with_suffix('.native.png')),1)
    final=OUT/'final-menu'
    names=['pokedex','dexnav','equipo','mochila','perfil','guardar','ajustes','claves','misiones']
    sheet([final/('rf-kanto-action-'+n+'.ppm') for n in names],'actions-dark-contact',3)
    sheet([final/('rf-kanto-return-'+n+'.ppm') for n in names],'returns-dark-contact',3)
    old=ROOT/'build_artifacts/ui/bag-user-sprites/rf-bag-reference-dark-quantities.ppm'
    new=OUT/'bag/rf-bag-reference-dark-quantities.ppm'
    sheet([old,new],'bag-before-after-native',2)
    convert(str(OUT/'bag-before-after-native.ppm'),str(OUT/'bag-before-after-2x.png'),2)
    print('Native 240x160 PNGs and before/after contact sheets exported without filtering.')


if __name__=='__main__': main()
