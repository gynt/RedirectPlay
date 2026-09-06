"""Compile the production ReceiveData method against a minimal queue fixture.

Requires a C++ compiler (cl in a developer shell, or CXX/g++ on other systems).
Does not need Steam, a login, network access or the private SDK submodules.
"""
from pathlib import Path
import os
import shlex
import subprocess
import tempfile

root=Path(__file__).resolve().parents[1]
source=(root/'RedirectPlay/ServiceProviders/Steamworks/Client/SteamPlayClient.cpp').read_text()
start=source.index('HRESULT CSteamPlayClient::ReceiveData(')
end=source.index('\nvoid CSteamPlayClient::ReceiveNetworkData()',start)
method=source[start:end]
fixture=(root/'tests/receive_data_fixture.cpp').read_text()
with tempfile.TemporaryDirectory(prefix='redirectplay-receive-') as directory:
    folder=Path(directory)
    unit=folder/'receive.cpp'; unit.write_text(fixture.replace('// PRODUCTION_RECEIVE_DATA',method))
    program=folder/('receive.exe' if os.name=='nt' else 'receive')
    compiler=os.environ.get('CXX', 'cl' if os.name=='nt' else 'g++')
    if Path(compiler).name.lower() in ('cl','cl.exe'):
        args=[compiler,'/nologo','/EHsc','/MT','/W4',str(unit),f'/Fe{program}',f'/Fo{folder/"receive.obj"}']
    else:
        args=shlex.split(compiler)+['-Wall','-Wextra',str(unit),'-o',str(program)]
    subprocess.run(args,cwd=folder,check=True)
    subprocess.run([str(program)],cwd=folder,check=True)
