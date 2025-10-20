type Ambiguous<W, N> = W | N;
type Platform = 'web' | 'native';
type FromAmbiguous<P extends Platform, A> =
  A extends Ambiguous<infer W, infer N> ? (P extends 'web' ? W : N) : never;

// type utils
type CommonKeys<W, N> = {
  [K in keyof W & keyof N]: [W[K]] extends [N[K]]
    ? [N[K]] extends [W[K]]
      ? K
      : never
    : never;
}[keyof W & keyof N];

export type CommonPart<W, N> = Pick<W, CommonKeys<W, N>> &
  Pick<N, CommonKeys<W, N>>;

interface MobileHaluluInterface {
  name: string;
  age: number;
  isCute: boolean;
  sayHello: () => string;
  play: (toy: string) => string;
  sleep: () => void;
  eat: (food: string) => void;
  playWith: (friend: MobileHaluluInterface) => string;
  makeFriend: (friend: MobileHaluluInterface) => string;
  getFriends: () => MobileHaluluInterface[];
  birthday: () => void;
  getAgeInDogYears: () => number;
  setName: (name: string) => void;
  setAge: (age: number) => void;
  setIsCute: (isCute: boolean) => void;
}

interface WebHaluluInterface {
  name: string;
  age: number;
  sayHello: () => string;
  play: (toy: string) => string;
  eat: (food: string) => void;
  playWith: (friend: WebHaluluInterface) => string;
  makeFriend: (friend: WebHaluluInterface) => string;
  getFriends: () => WebHaluluInterface[];
  birthday: () => void;
  getAgeInDogYears: () => number;
  setName: (name: string) => void;
  setAge: (age: number) => void;
  fetchFromServer: (url: string) => Promise<void>;
  saveToServer: (url: string) => Promise<void>;
}

type AHalulu = Ambiguous<MobileHaluluInterface, WebHaluluInterface>;
type CHalulu = CommonPart<MobileHaluluInterface, WebHaluluInterface>;

type Halulu = AHalulu & CHalulu;

interface IHalulu {
  name: string;
  age: number;
  isCute: boolean;
  sayHello: () => string;
  play: (toy: string) => string;
  sleep: () => void;
  eat: (food: string) => void;
  playWith: (friend: MobileHaluluInterface) => string;
  makeFriend: (friend: MobileHaluluInterface) => string;
  getFriends: () => MobileHaluluInterface[];
  birthday: () => void;
  getAgeInDogYears: () => number;
  setName: (name: string) => void;
  setAge: (age: number) => void;
  setIsCute: (isCute: boolean) => void;
  fetchFromServer: (url: string) => Promise<void>;
  saveToServer: (url: string) => Promise<void>;
}

type SelfWeb<T> = FromAmbiguous<'web', T>;

export class HaluluWeb implements IHalulu {
  private internalHalulu: Halulu;

  constructor(internalHalulu: Halulu) {
    this.internalHalulu = internalHalulu;
  }

  public get name(): string {
    return this.internalHalulu.name;
  }

  public get age(): number {
    return this.internalHalulu.age;
  }

  public get isCute(): boolean {
    return false;
  }

  public birthday(): void {}

  public getAgeInDogYears(): number {
    return this.internalHalulu.getAgeInDogYears();
  }

  public setIsCute(_isCute: boolean): void {
    console.warn('isCute is not available on web');
  }

  public setName(name: string): void {
    this.internalHalulu.setName(name);
  }

  public setAge(age: number): void {
    this.internalHalulu.setAge(age);
  }

  public getFriends(): MobileHaluluInterface[] {
    return [];
  }

  public sayHello(): string {
    return this.internalHalulu.sayHello();
  }

  public play(toy: string): string {
    return this.internalHalulu.play(toy);
  }

  public sleep(): void {
    console.warn('HaluluWeb cannot sleep');
  }

  public eat(food: string): void {
    return this.internalHalulu.eat(food);
  }

  public playWith(friend: Halulu): string {
    const self = this.internalHalulu as SelfWeb<AHalulu>;
    const other = (friend as unknown as HaluluWeb)
      .internalHalulu as SelfWeb<AHalulu>;

    return self.playWith(other as unknown as AHalulu);
  }

  public makeFriend(friend: Halulu): string {
    return this.internalHalulu.makeFriend(friend.internalHalulu);
  }

  public fetchFromServer(url: string): Promise<void> {
    return (this.internalHalulu as WebHaluluInterface).fetchFromServer(url);
  }

  public saveToServer(url: string): Promise<void> {
    return (this.internalHalulu as WebHaluluInterface).saveToServer(url);
  }
}

export class HaluluMobile implements IHalulu {
  private internalHalulu: Halulu;

  constructor(internalHalulu: Halulu) {
    this.internalHalulu = internalHalulu;
  }

  public getName(): string {
    return this.internalHalulu.name;
  }

  public makeFriend(friend: HaluluMobile): string {
    return this.internalHalulu.makeFriend(
      friend.internalHalulu as MobileHaluluInterface
    );
  }

  saveToServer(_url: string): Promise<void> {
    throw new Error('saveToServer is not available on mobile');
  }
}
