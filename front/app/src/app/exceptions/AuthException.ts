export class AuthException extends Error {
    constructor(message: string) {
      super(message); 
      this.name = 'AuthException'; 

      Object.setPrototypeOf(this, AuthException.prototype);
      console.log(message);
    }
  }
  