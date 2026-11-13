import sys,json,os,time
from pathlib import Path
root=Path(__file__).resolve().parents[1]
output=root/'build-sim/settings-test';output.mkdir(parents=True,exist_ok=True)
sys.path.insert(0,str(root/'managed_components/espressif__esp-gsp/tools/sim_bridge'))
from run import simulator_command,start,wait_ready,stop
from rpc_client import connect
manifest=json.loads((root/'build-sim/clock_backend-Release.json').read_text())
ready=Path('/tmp/flip-clock-sim-ready.json');ready.unlink(missing_ok=True)
host=[os.environ.get('MOSAICO_SIM', str(Path.home()/'.cache/esp-mosaico/sim/1.5.1/sim'))]
sim=start([*host,'--bundle',manifest['bundle'],'--frames','0','--headless','--api-json',str(root/'build-sim/clock_backend_assets/bundle_gsp.api.json'),'--backend-enable','--backend-required','--api-listen','tcp://127.0.0.1:8266','--ready-file',str(ready)])
backend=None
try:
 info=wait_ready(sim,ready,90,managed=False)
 os.environ['FLIP_CLOCK_DEMO']='1'
 import subprocess
 backend_log=open(output/'backend.log','w')
 backend=subprocess.Popen([manifest['executable'],'--endpoint',info['backend'],'--duration-ms','24000'],start_new_session=True,stdout=backend_log,stderr=subprocess.STDOUT)
 with connect(port=8266) as api:
  from PIL import Image
  time.sleep(.4)
  assert 'error' not in api.call('feed_pointer',{'x':240,'y':240,'pressed':True})
  time.sleep(1.1)
  api.call('feed_pointer',{'x':240,'y':240,'pressed':False})
  time.sleep(.25)
  api.call('screenshot',{'path':str(output/'menu.png'),'format':'png'})
  im=Image.open(str(output/'menu.png')).convert('RGB')
  bright=sum(max(im.getpixel((x,y)))>180 for x in range(110,350) for y in range(15,65))
  assert bright>250, 'Long press must open Settings'
  def tap(name):
   result=api.call('tap_object',{'name':name});assert 'error' not in result,result
   time.sleep(.12)
  def shot(name):
   time.sleep(.3)
   assert 'error' not in api.call('screenshot',{'path':str(output/(name+'.png')),'format':'png'})
  def word(value):
   for char in value:
    for row,letters in enumerate(['qwertyuiop','asdfghjkl','zxcvbnm']):
     if char in letters:
      tap('password_keys_lower_key_%d_%d'%(row,letters.index(char)+(1 if row==2 else 0)));break
  tap('wifi_option');shot('networks')
  api.call('tap',{'x':150,'y':184});shot('saved')
  api.call('tap',{'x':150,'y':248});shot('password')
  word('wrongpass');shot('masked')
  tap('password_keys_lower_key_3_2');shot('failure')
  api.call('tap',{'x':150,'y':248});time.sleep(.2)
  word('password');tap('password_keys_lower_key_3_2');shot('connected')
  api.call('tap',{'x':150,'y':312});shot('open')
  tap('back');tap('back');shot('clock')
  print('PASS: long press, saved/new/open selection, password entry, retry, Back navigation')
 backend.wait(timeout=25)
 assert backend.returncode==0,backend.returncode
 backend_log.close()
 results=[line for line in (output/'backend.log').read_text().splitlines() if line.startswith('wifi_sim:')]
 assert results==['wifi_sim: connected via saved','wifi_sim: failed via password','wifi_sim: connected via password','wifi_sim: connected via open'],results
 print('PASS: verified exact saved/failure/retry/open service outcomes')
finally:

 for process in (backend,sim):
  if process and process.poll() is None: stop(process,tree=True)
