#!/usr/bin/env python3

import logging
import os
import subprocess
import sys
import time
import ROOT

# spill_length = 22406920
spill_length = 55406920
error_length =    10000

#scaler_dir = os.path.abspath('/data3/E42SubData/scaler_2021may')
scaler_dir = os.path.abspath('/misc/scaler_e42_2021may')
scaler_list = [os.path.join(scaler_dir, 'hul01scr-1.txt'),
               os.path.join(scaler_dir, 'hul01scr-2.txt')]
scaler_line = [96, 32]
output_dir = '/data3/E42SubData/scaler_2021may'

channel_map = {
  # 'name': [hul, channel]
  'BH1': [0, 16],
  'BH1-SUM': [-1, -1],
  'BH1-01': [1, 0],
  'BH1-02': [1, 1],
  'BH1-03': [1, 2],
  'BH1-04': [1, 3],
  'BH1-05': [1, 4],
  'BH1-06': [1, 5],
  'BH1-07': [1, 6],
  'BH1-08': [1, 7],
  'BH1-09': [1, 8],
  'BH1-10': [1, 9],
  'BH1-11': [1, 10],
  'BH2': [0, 17],
  'BH2-SUM': [-1, -1],
  'BH2-01': [0, 64],
  'BH2-02': [0, 65],
  'BH2-03': [0, 66],
  'BH2-04': [0, 67],
  'BH2-05': [0, 68],
  'BH2-06': [0, 69],
  'BH2-07': [0, 70],
  'BH2-08': [0, 71],
  'BAC': [0, 18],
  'HTOF': [0, 19],
  'SCH': [0, 26],
  'Beam': [0, 38],
  'K-Beam': [0, 35],
  'Pi-Beam': [0, 40],
  'TM': [0, 9],
  'SY': [0, 10],
  'BH1-1/100-PS': [1, 11],
  'BH1-1/1e5-PS': [1, 12],
  'TOF-24': [0, 29],
  'HTOF-Cosmic': [0, 25],
  # 'Other2': [0, 26],
  # 'Other3': [0, 27],
  # 'Other4': [0, 28],
  'BEAM-A': [0, 35],
  'BEAM-B': [0, 36],
  'BEAM-C': [0, 37],
  'BEAM-D': [0, 38],
  'BEAM-E': [0, 39],
  'BEAM-F': [0, 40],
  'Mtx2D-1': [0, 32],
  'Mtx2D-2': [0, 33],
  'Mtx3D': [0, 34],
  'HTOF-Mp2': [0, 20],
  'HTOF-Mp3': [0, 21],
  'HTOF-Mp4': [0, 27],
  'HTOF-Mp5': [0, 28],
  'BVH': [0, 13],
  'TOF': [0, 22],
  'LAC': [0, 23],
  'WC': [0, 24],
  'Spill': [-1, -1],
  '10M-Clock': [0, 0],
  'Real-Time': [0, 1],
  'Live-Time': [0, 2],
  'L1-Req': [0, 3],
  'L1-Acc': [0, 4],
  # 'MstClr': [0, 5],
  # 'Clear': [0, 6],
  'L2-Req': [0, 7],
  'L2-Acc': [0, 8],
  'GET-Busy': [0, 14],
  'TRIG-A': [0, 41],
  'TRIG-B': [0, 42],
  'TRIG-C': [0, 43],
  'TRIG-D': [0, 44],
  'TRIG-E': [0, 45],
  'TRIG-F': [0, 46],
  'TRIG-A-PS': [0, 48],
  'TRIG-B-PS': [0, 49],
  'TRIG-C-PS': [0, 50],
  'TRIG-D-PS': [0, 51],
  'TRIG-E-PS': [0, 52],
  'TRIG-F-PS': [0, 53],
  'TRIG-PSOR-A': [0, 54],
  'TRIG-PSOR-B': [0, 55],
  'Clock-PS': [0, 56],
  'Reserve2-PS': [0, 57],
  'Level1-PS': [0, 58],
}

#inverse_map = { str(v): k for k, v in channel_map.items() }

#______________________________________________________________________________
def find_key(val):
  ret = list()
  for k, v in channel_map.items():
    if val == v:
      ret.append(k)
  return ret

#______________________________________________________________________________
def separate_comma(value):
  value = int(value)
  sep_num = list()
  while int(value/1000) > 0:
    sep_num.append(value%1000)
    value = int(value/1000)
  ret = str(value)
  for itr in reversed(sep_num):
    ret += f',{itr:03d}'
  return ret

#______________________________________________________________________________
class ScalerAnalyzer():

  #____________________________________________________________________________
  def __init__(self, port=9090):
    ROOT.gROOT.SetBatch()
    ROOT.gErrorIgnoreLevel = ROOT.kFatal
    ROOT.gSystem.Setenv('AUTO_SCALER_PRINT', '0')
    ROOT.gSystem.Setenv('SCALER_PRINT', '0')
    self.port = port
    self.server = ROOT.THttpServer(f'http:{self.port}?loopback?thrds=5')
    self.server.Restrict('/', 'allow=all')
    self.server.CreateItem('/', 'Online Scaler')
    self.server.SetJSROOT('https://root.cern.ch/js/latest/')
    command = ('if(TString(gSystem->Getenv("AUTO_SCALER_PRINT"))=="0")' +
               '{gSystem->Setenv("AUTO_SCALER_PRINT", "1");}else{' +
               'gSystem->Setenv("AUTO_SCALER_PRINT", "0");}')
    self.server.RegisterCommand('/Switch_Auto_Print_Mode', command)
    command = 'gSystem->Setenv("SCALER_PRINT", "1")'
    self.server.RegisterCommand('/Print_10_Spill', command)
    self.server.SetItemField('/','_monitoring','200')
    # self.server.SetItemField('/','_layout','vert2')
    # self.server.SetItemField('/','_drawitem','[Spill,Canvases/Trend]')
    self.server.SetItemField('/','_drawitem','Spill')
    # self.server.SetItemField('/','_drawitem','Scaler')
    self.server.CreateItem('/Spill', 'Online Scaler')
    self.server.SetItemField('/Spill', '_kind', 'Text')
    self.server.CreateItem('/Run', 'Online Scaler')
    self.server.SetItemField('/Run', '_kind', 'Text')
    # self.server.CreateItem('/Tag', 'Tag Check')
    # self.server.SetItemField('/Tag', '_kind', 'Text')
    # content = ('<div style="color: white; background-color: black;' +
    #            'width: 100%; height: 100%;">' +
    #            'Tag cheker is running' + '</div>')
    # self.server.SetItemField('/Tag', 'value', content)
    self.now = ROOT.TTimeStamp()
    self.now.Add(-self.now.GetZoneOffset())
    self.spill_end = False
    self.ignore = False
    self.runno = 0
    self.evno = 0
    self.prev_runno = 0
    self.prev_evno = 0
    self.scaler = dict()
    self.scaler_prev = dict()
    self.scaler_runsum = dict()
    self.scaler_tmp = dict()
    for k, v in channel_map.items():
      self.scaler[k] = 0
      self.scaler_prev[k] = 0
      self.scaler_runsum[k] = 0
      self.scaler_tmp[k] = 0
    self.footer = self.calculate_footer(self.scaler)
    self.footer_runsum = self.calculate_footer(self.scaler_runsum)
    self.footer_tmp = self.calculate_footer(self.scaler_tmp)
    self.nraw = 26
    self.runnos = [-1, -2]
    self.evnos = [-1, -2]
    self.auto_print_mode = False
    self.print_flag = False
    self.print_spill = 0
    self.print_n_spill = 10
    ''' graph '''
    self.nplot = int(3600*24/5.2)
    self.plot_unixtime = list()
    for i in range(self.nplot):
      self.plot_unixtime.append(self.now.GetSec() + 5.2*(i - self.nplot))
    self.graph_beam = list()
    self.plot_beam = list()
    canvas_trend = ROOT.TCanvas('Trend', 'Trend')
    canvas_trend.Divide(2, 1)
    self.server.Register('/', canvas_trend)
    self.server.Hide('/Trend')
    canvas_trend.cd(1).SetGridy()
    self.legend_beam = ROOT.TLegend(0.130, 0.130, 0.280, 0.410)
    self.legend_beam.SetTextSize(0.05)
    self.legend_beam.SetFillColor(0)
    self.legend_beam.SetBorderSize(4)
    for i, l in enumerate(['Beam', 'K-Beam', '#pi-Beam']):
      self.plot_beam.append([0 for i in range(self.nplot)])
      g = ROOT.TGraph()
      g.SetName(l)
      g.SetTitle('Beam')
      g.SetMarkerStyle(8)
      g.SetMarkerSize(1.5)
      g.SetMarkerColor([ROOT.kBlack, ROOT.kGreen+1, ROOT.kBlue+1][i])
      g.SetLineWidth(3)
      g.SetLineColor([ROOT.kBlack, ROOT.kGreen+1, ROOT.kBlue+1][i])
      g.GetXaxis().SetTimeDisplay(1)
      g.GetXaxis().SetTimeFormat('%m/%d %H:%M')
      g.GetXaxis().SetTimeOffset(self.now.GetZoneOffset(), 'jpb')
      # g.GetXaxis().SetLabelOffset(0.04)
      g.SetPoint(0, 0, 0)
      if i == 0:
        g.Draw('AL')
      else:
        g.Draw('L')
      self.graph_beam.append(g)
      self.legend_beam.AddEntry(g, l, 'P')
    self.legend_beam.SetHeader(f'K/#pi Ratio : {0:5.3f}')
    self.legend_beam.Draw()
    self.graph_daq = list()
    self.plot_daq = list()
    canvas_trend.cd(2).SetGridy()
    self.legend_daq = ROOT.TLegend(0.130, 0.130, 0.280, 0.410)
    self.legend_daq.SetTextSize(0.05)
    self.legend_daq.SetFillColor(0)
    self.legend_daq.SetBorderSize(4)
    for i, l in enumerate(['DAQ-Eff', 'L2-Eff', 'Duty']):
      self.plot_daq.append([0 for i in range(self.nplot)])
      g = ROOT.TGraph()
      g.SetName(l)
      g.SetTitle('DAQ')
      g.SetMarkerStyle(8)
      g.SetMarkerSize(1.5)
      g.SetMarkerColor([ROOT.kRed+1, ROOT.kOrange+1, ROOT.kBlue+1][i])
      g.SetLineWidth(3)
      g.SetLineColor([ROOT.kRed+1, ROOT.kOrange+1, ROOT.kBlue+1][i])
      g.GetXaxis().SetTimeDisplay(1)
      g.GetXaxis().SetTimeFormat('%m/%d %H:%M')
      g.GetXaxis().SetTimeOffset(self.now.GetZoneOffset(), 'jpb')
      # g.GetXaxis().SetLabelOffset(0.04)
      g.SetPoint(0, 0, 0)
      if i == 0:
        g.Draw('AL')
      else:
        g.Draw('L')
      self.graph_daq.append(g)
      self.legend_daq.AddEntry(g, l, 'P')
    self.legend_daq.SetHeader(f'DAQ Eff. : {0:5.3f}')
    self.legend_daq.Draw()

  #____________________________________________________________________________
  def add_runsum(self, scaler):
    for k, v in channel_map.items():
      self.scaler_runsum[k] += scaler[k]
    self.scaler_runsum['Spill'] = scaler['Spill']
    self.footer_runsum = self.calculate_footer(self.scaler_runsum)

  #____________________________________________________________________________
  def add_tmp(self):
    for k, v in channel_map.items():
      self.scaler_tmp[k] += self.scaler[k]
    self.scaler_tmp['Spill'] = self.scaler['Spill']
    self.footer_tmp = self.calculate_footer(self.scaler_tmp)

 #____________________________________________________________________________
  def calculate_footer(self, scaler_dict):
    ret = dict()
    ret['Beam/TM'] = (
      scaler_dict['Beam']/scaler_dict['TM'] if scaler_dict['TM'] > 0
      else ROOT.TMath.QuietNaN())
    ret['Live/Real'] = (
      scaler_dict['Live-Time']/scaler_dict['Real-Time']
      if scaler_dict['Real-Time'] > 0
      else ROOT.TMath.QuietNaN())
    ret['DAQ-Eff'] = (
      scaler_dict['L1-Acc']/scaler_dict['L1-Req']
      if scaler_dict['L1-Req'] > 0
      else ROOT.TMath.QuietNaN())
    ret['L1Req/Beam'] = (
      scaler_dict['L1-Req']/scaler_dict['Beam']
      if scaler_dict['Beam'] > 0
      else ROOT.TMath.QuietNaN())
    ret['L2-Eff'] = (
      scaler_dict['L2-Acc']/scaler_dict['L1-Acc']
      if scaler_dict['L1-Acc'] > 0
      else ROOT.TMath.QuietNaN())
    ret['Duty-Factor'] = min(
      ret['DAQ-Eff'] / (1. - ret['DAQ-Eff']) *
      (1./ret['Live/Real'] - 1)
      if (ret['Live/Real'] > 0 and ret['DAQ-Eff'] < 1.)
      else 1., 1.)
    return ret

  #____________________________________________________________________________
  def draw_one_box(self, x, y, title, val):
    if title == 'Pi-Beam':
      title = '#pi-Beam'
    if title == 'Spill':
      title = 'Spill#'
    tex = ROOT.TLatex()
    tex.SetNDC()
    tex.SetTextSize(0.04)
    tex.SetTextAlign(12)
    tex.DrawLatex(x, y, title)
    tex.SetTextAlign(32)
    if 0.1 < x and x < 0.5:
      tex.DrawLatex(x + 0.295, y, val)
    else:
      tex.DrawLatex(x + 0.280, y, val)

  #____________________________________________________________________________
  def draw_one_line(self, line_no, info_array, footer=False):
    ystep = 0.05
    y0 = 0.95
    if len(info_array) == 3 and footer:
      self.draw_one_line(
        line_no,
        [info_array[0], f'{self.footer_tmp[info_array[0]]:.6f}',
         info_array[1], f'{self.footer_tmp[info_array[1]]:.6f}',
         info_array[2], f'{self.footer_tmp[info_array[2]]:.6f}'])
    elif len(info_array) == 3 and not footer:
      self.draw_one_line(line_no, [info_array[0], self.get(info_array[0]),
                                   info_array[1], self.get(info_array[1]),
                                   info_array[2], self.get(info_array[2])])
    elif len(info_array) == 6:
      y = y0 - ystep*line_no
      x = [0.05, 0.35, 0.67]
      for i in range(3):
        self.draw_one_box(x[i], y, info_array[2*i], info_array[2*i + 1])
      line = ROOT.TLine()
      line.SetNDC()
      line.SetLineColor(ROOT.kGray)
      line.DrawLine(0.05, y-0.5*ystep, 0.95, y-0.5*ystep)
      line.SetLineColor(ROOT.kBlack)
      line.DrawLine(0.34, y-0.5*ystep, 0.34, y+0.5*ystep)
      line.DrawLine(0.66, y-0.5*ystep, 0.66, y+0.5*ystep)
      if (line_no == 1 or line_no == 5 or line_no ==14):
        line.DrawLine(0.05, y-0.5*ystep, 0.95, y-0.5*ystep)

  #____________________________________________________________________________
  def print_scaler_sheet(self, file_name, lpr=True):
    c1 = ROOT.TCanvas('c1', 'c1', 1200, 800)
    self.draw_one_line(1, [self.now.AsString('s'), '',
                           'Event#', separate_comma(self.evno),
                           '#color[' + f'{ROOT.kRed+1}' + ']{Run#}',
                           separate_comma(self.runno)])
    self.draw_one_line(2, ['Spill', 'TM', '10M-Clock'])
    self.draw_one_line(3, ['Beam', 'SY', 'BH1-1/100-PS'])
    self.draw_one_line(4, ['K-Beam', 'BH1-SUM', 'BH1-1/1e5-PS'])
    self.draw_one_line(5, ['Pi-Beam', 'BH2-SUM', 'TOF-24'])
    self.draw_one_line(6, ['BH1', 'TRIG-A-PS', 'L1-Req'])
    self.draw_one_line(7, ['BH2', 'TRIG-B-PS', 'L1-Acc'])
    self.draw_one_line(8, ['BAC', 'TRIG-C-PS', 'Clear'])
    self.draw_one_line(9, ['HTOF', 'TRIG-D-PS', 'L2-Req'])
    self.draw_one_line(10, ['SCH', 'TRIG-E-PS', 'L2-Acc'])
    self.draw_one_line(11, ['TOF', 'TRIG-F-PS', 'HTOF-Mp2'])
    self.draw_one_line(12, ['BVH', 'Mtx2D-1', 'HTOF-Mp3'])
    self.draw_one_line(13, ['LAC', 'Mtx2D-2', 'HTOF-Mp4'])
    self.draw_one_line(14, ['WC', 'Mtx3D', 'HTOF-Mp5'])
    self.draw_one_line(15, ['Beam/TM', 'Live/Real', 'DAQ-Eff'],
                       footer=True)
    self.draw_one_line(16, ['L1Req/Beam', 'L2-Eff', 'Duty-Factor'],
                       footer=True)
    output_path = os.path.join(output_dir, file_name)
    c1.Print(output_path)
    if lpr:
      subprocess.run(['lpr', output_path])
    logging.info(f'print {output_path}')
    for k, v in channel_map.items():
      self.scaler_tmp[k] = 0
    self.print_spill = 0
    self.print_flag = False
    ROOT.gSystem.Setenv('SCALER_PRINT', '0')

  #____________________________________________________________________________
  def get(self, key):
    return (separate_comma(self.scaler_tmp[key])
            if key in self.scaler_tmp else '')

  #____________________________________________________________________________
  def make_summary(self, file_name, scaler_list, footer_list):
    output_path = os.path.join(output_dir, file_name)
    buf = str()
    with open(output_path, 'w') as f:
      buf += ('# Generated by ' + os.path.abspath(sys.argv[0]) +
              ' on ' + self.now.AsString('s') + '\n\n')
      buf += f'{"Run":<20}{self.prev_runno:>20}\n'
      buf += f'{"Event":<20}{self.prev_evno:>20}\n'
      for key, val in scaler_list.items():
        if key == 'n/a':
          continue
        buf += f'{key:<20}{val:>20}\n'
      buf += '\n'
      for key, val in footer_list.items():
        buf += f'{key:<20}{val:>20.9f}\n'
      f.write(buf)

  #____________________________________________________________________________
  def run(self):
    while not ROOT.gSystem.ProcessEvents():
      self.now = ROOT.TTimeStamp()
      self.now.Add(-self.now.GetZoneOffset())
      self.auto_print_mode = (ROOT.gSystem.Getenv('AUTO_SCALER_PRINT') == '1')
      self.print_flag = (ROOT.gSystem.Getenv('SCALER_PRINT') == '1')
      self.update_scaler()
      self.update_summary()
      self.update_tmp()
      self.update_content('/Spill', self.scaler, self.footer)
      self.update_content('/Run', self.scaler_runsum, self.footer_runsum)
      self.update_graph()
      time.sleep(0.1)

  #____________________________________________________________________________
  def update_content(self, label, scaler_dict, footer_dict):
    if self.ignore:
      return
    i = 0
    disp_left = list()
    disp_center = list()
    disp_right = list()
    disp_left.append(['RUN', separate_comma(self.runno)])
    disp_center.append(['Event Number', separate_comma(self.evno)])
    disp_right.append(['', 'Spill End' if self.spill_end else ''])
    disp_left.append(['Auto Print Mode',
                      'ON' if self.auto_print_mode else 'OFF'])
    disp_center.append(['', ''])
    if self.print_flag:
      disp_right.append(['Printing ...',
                         f'{self.print_spill}/{self.print_n_spill}'])
    else:
      disp_right.append(['', ''])
    for k, v in scaler_dict.items():
      if i < self.nraw:
        disp_left.append([k, separate_comma(v)])
      elif self.nraw <= i and i < self.nraw*2:
        disp_center.append([k, separate_comma(v)])
      elif self.nraw*2 <= i and i < self.nraw*3:
        disp_right.append([k, separate_comma(v)])
      i += 1
    content = ('<div style="color: white; background-color: black;' +
               'width: 100%; height: 100%;">' +
               '<table border="0" width="700" cellpadding="0">')
    for i in range(self.nraw+2):
      if i == len(disp_left):
        disp_left.append(['n/a', '0'])
      if i == len(disp_center):
        disp_center.append(['n/a', '0'])
      if i == len(disp_right):
        disp_right.append(['n/a', '0'])
      content += '<tr>'
      content += (f'<td width="100">{disp_left[i][0]}</td>' +
                  f'<td align="right" width="100">{disp_left[i][1]}</td>')
      content += (f'<td width="100"> : {disp_center[i][0]}</td>' +
                  f'<td align="right" width="100">{disp_center[i][1]}</td>')
      content += (f'<td width="100"> : {disp_right[i][0]}</td>' +
                  f'<td align="right" width="100">{disp_right[i][1]}</td>')
      content += '</tr>'
    content += '<tr>'
    content += '<td width="100"></td><td align="right" width="100"></td>'
    content += '<td width="100"> : </td><td align="right" width="100"></td>'
    content += '<td width="100"> : </td><td align="right" width="100"></td>'
    content += '</tr>'
    content += '<tr>'
    content += ('<td width="100">Beam/TM</td>' +
                '<td align="right" width="100">' +
                f'{footer_dict["Beam/TM"]:.6f}</td>')
    content += ('<td width="100"> : Live/Real</td>' +
                '<td align="right" width="100">' +
                f'{footer_dict["Live/Real"]:.6f}</td>')
    content += ('<td width="100"> : DAQ-Eff</td>' +
                '<td align="right" width="100">' +
                f'{footer_dict["DAQ-Eff"]:.6f}</td>')
    content += '</tr>'
    content += '<tr>'
    content += ('<td width="100">L1Req/Beam</td>' +
                '<td align="right" width="100">' +
                f'{footer_dict["L1Req/Beam"]:.6f}</td>')
    content += ('<td width="100"> : L2-Eff</td>' +
                '<td align="right" width="100">' +
                f'{footer_dict["L2-Eff"]:.6f}</td>')
    content += ('<td width="100"> : Duty-Factor</td>' +
                '<td align="right" width="100">' +
                f'{footer_dict["Duty-Factor"]:.6f}</td>')
    content += '</tr>'
    content += '</table></div>'
    self.server.SetItemField(label, 'value', content)

  #____________________________________________________________________________
  def update_graph(self):
    if not self.spill_end:
      return
    kpi = (self.scaler['K-Beam']/self.scaler['Pi-Beam']
           if self.scaler['Pi-Beam'] > 0 else ROOT.TMath.QuietNaN())
    self.legend_beam.SetHeader(f'K/#pi Ratio : {kpi:5.3f}')
    values = [self.scaler['Beam'],
              self.scaler['K-Beam'],
              self.scaler['Pi-Beam']]
    self.plot_unixtime.pop(0)
    self.plot_unixtime.append(self.now.GetSec())
    max_count = 1
    for i, g in enumerate(self.graph_beam):
      self.plot_beam[i].pop(0)
      self.plot_beam[i].append(values[i])
      g.Set(0)
      for j in range(self.nplot):
        g.SetPoint(j, self.plot_unixtime[j], self.plot_beam[i][j])
        max_count = max(self.plot_beam[i][j], max_count)
      g.GetYaxis().SetRangeUser(0, max_count*1.1)
      g.GetXaxis().SetLimits(self.plot_unixtime[-1]+600-7200,
                             self.plot_unixtime[-1]+600)
    self.legend_daq.SetHeader(f'DAQ Eff : {self.footer["DAQ-Eff"]:5.3f}')
    values = [self.footer['DAQ-Eff'],
              self.footer['L2-Eff'],
              self.footer['Duty-Factor']]
    for i, g in enumerate(self.graph_daq):
      self.plot_daq[i].pop(0)
      self.plot_daq[i].append(values[i])
      g.Set(0)
      for j in range(self.nplot):
        g.SetPoint(j, self.plot_unixtime[j], self.plot_daq[i][j])
      g.GetYaxis().SetRangeUser(0, 1.05)
      g.GetXaxis().SetLimits(self.plot_unixtime[-1]+600-7200,
                             self.plot_unixtime[-1]+600)

  #____________________________________________________________________________
  def update_scaler(self):
    for i, scaler_txt in enumerate(scaler_list):
      n_line = 0
      with open(scaler_txt, 'r') as f:
        columns = f.readline().split()
        if len(columns) != 2:
          continue
        self.runnos[i] = int(columns[0])
        self.evnos[i] = int(columns[1])
        for line in f.readlines():
          n_line += 1
          columns = line.split()
          if len(columns) != 2:
            continue
          index = [i, int(columns[0])]
          for key in find_key(index):
            self.scaler[key] = int(columns[1])
        if n_line != scaler_line[i]:
          logging.warning(f'{scaler_txt} is broken')
          self.spill_end = False
          self.ignore = True
          return
    # BH1 SUM
    self.scaler['BH1-SUM'] = 0
    for i in range(11):
      self.scaler['BH1-SUM'] += self.scaler[f'BH1-{i+1:02d}']
    # BH2 SUM
    self.scaler['BH2-SUM'] = 0
    for i in range(8):
      self.scaler['BH2-SUM'] += self.scaler[f'BH2-{i+1:02d}']
    self.runno = max(self.runnos[0], self.runnos[1])
    self.evno = max(self.evnos[0], self.evnos[1]) + 1
    if self.evno == self.prev_evno:
      self.spill_end = False
      self.ignore = True
      return
    if self.runno != self.prev_runno and self.prev_runno != 0:
      self.add_runsum(self.scaler_prev)
      self.make_summary(f'scaler_{self.prev_runno:05d}.txt',
                        self.scaler_runsum,
                        self.footer_runsum)
      self.scaler['Spill'] = 0
      for k, v in channel_map.items():
        self.scaler_runsum[k] = 0
    self.spill_end = (
      self.runnos[0] == self.runnos[1] and
      self.evnos[0] == self.evnos[1] and
      abs(self.scaler['10M-Clock'] - spill_length) < error_length)
    if self.spill_end:
      self.scaler['Spill'] += 1
      self.add_runsum(self.scaler)
    self.footer = self.calculate_footer(self.scaler)
    if self.auto_print_mode and self.scaler['Spill'] == 2:
      ROOT.gSystem.Setenv('SCALER_PRINT', '1')
    self.prev_runno = self.runno
    self.prev_evno = self.evno
    self.scaler_prev = self.scaler
    self.ignore = False

  #____________________________________________________________________________
  def update_summary(self):
    if self.spill_end:
      self.make_summary('spill_end.txt', self.scaler, self.footer)

  #____________________________________________________________________________
  def update_tmp(self):
    if self.spill_end and self.print_flag:
      self.print_spill += 1
      self.add_tmp()
      if self.print_spill == self.print_n_spill:
        self.print_scaler_sheet('scaler_sheet_tmp.pdf')

#______________________________________________________________________________
if __name__ == '__main__':
  formatter = '%(asctime)s [%(levelname)s] %(message)s'
  logging.basicConfig(level=logging.DEBUG,
                      format=formatter)
  logging.info('start scaler')
  try:
    ana = ScalerAnalyzer(port=9092)
    ana.run()
  except KeyboardInterrupt:
    print()
    logging.info('catch SIGINT')
    logging.info('stop scaler')
    ROOT.gSystem.Exit(0)
