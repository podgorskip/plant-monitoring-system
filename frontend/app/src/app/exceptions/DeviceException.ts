export class DeviceException extends Error {
    constructor(message: string) {
      super(message); 
      this.name = 'DeviceException'; 

      Object.setPrototypeOf(this, DeviceException.prototype);
      console.log(message);
    }
  }
  