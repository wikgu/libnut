const libnut = require("..");

describe("Screen", () => {
  it("Get screen size.", function() {
    let screenSize;
    expect((screenSize = libnut.getScreenSize())).toBeTruthy();
    expect(screenSize.width !== undefined).toBeTruthy();
    expect(screenSize.height !== undefined).toBeTruthy();
  });
});

describe("Capture", () => {
  it("byteWidth", () => {
    // GIVEN
    const regionScreenShot = libnut.screen.capture(0, 0, 100, 100);

    // WHEN
    const expectedByteWidth = regionScreenShot.width * regionScreenShot.bytesPerPixel;

    // THEN
    expect(expectedByteWidth).toBe(regionScreenShot.byteWidth);
  });
});
