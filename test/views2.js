// WebBrowserViews in scroll, use setContentBaseView for BaseWindow

const path = require("path");
const { app, BaseWindow, View, WebContentsView } = require("electron");

global.win = null;

function createWindow () {
  win = new BaseWindow({ width: 1000, height: 500 })

  const view0 = new View()
  win.contentView.addChildView(view0)
  view0.setBounds({ x: 0, y: 0, width: 1000, height: 400 })

  const view1 = new WebContentsView()
  view0.addChildView(view1)
  view1.webContents.loadURL('https://bitbucket.org')
  view1.setBounds({ x: 0, y: 0, width: 400, height: 400 })

  const view2 = new WebContentsView()
  view0.addChildView(view2)
  view2.webContents.loadURL('https://github.com')
  view2.setBounds({ x: 500, y: 0, width: 400, height: 400 })
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(() => {
  createWindow();

  app.on('activate', function () {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (BaseWindow.getAllWindows().length === 0)
      createWindow();
  });
});

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', function () {
  if (process.platform !== 'darwin')
    app.quit();
});