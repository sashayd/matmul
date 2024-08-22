import shutil

from precompiler import PrecompilerInstance

with open('kernel_sizes.txt', 'r') as f:
    kernel_size_list = eval(f.read(), dict())

with open('mm_template.c', 'r') as fin:
    for mr, nr in kernel_size_list:
        fin.seek(0)
        pc = PrecompilerInstance()
        pc.add_var('mr', mr)
        pc.add_var('nr', nr)
        result = pc(fin)
        with open(f'mm{mr}x{nr}.c', 'w') as fout:
            result.seek(0)
            shutil.copyfileobj(result, fout)

with open('mm_time_template.c', 'r') as fin:
    pc = PrecompilerInstance()
    result = pc(fin)
    with open(f'mm_time.c', 'w') as fout:
        result.seek(0)
        shutil.copyfileobj(result, fout)