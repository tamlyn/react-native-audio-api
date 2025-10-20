interface BaseHalulu<T> {
  name: string;
  age: number;
  sayHello: () => string;
  play: (toy: string) => string;
  eat: (food: string) => void;
  playWith: (friend: T) => string;
  makeFriend: (friend: T) => string;
  getFriends: () => T[];
  birthday: () => void;
  getAgeInDogYears: () => number;
  setName: (name: string) => void;
  setAge: (age: number) => void;
}

interface MobileHaluluInterface extends BaseHalulu<MobileHaluluInterface> {
  isCute: boolean;
  sleep: () => void;
  setIsCute: (isCute: boolean) => void;
}

interface WebHaluluInterface extends BaseHalulu<WebHaluluInterface> {
  fetchFromServer: (url: string) => Promise<void>;
  saveToServer: (url: string) => Promise<void>;
}

interface IHalulu<T> extends BaseHalulu<IHalulu<T>> {
  isCute: boolean;
  sleep: () => void;
  setIsCute: (isCute: boolean) => void;
  fetchFromServer: (url: string) => Promise<void>;
  saveToServer: (url: string) => Promise<void>;
}

abstract class HaluluCommon<I, IH extends BaseHalulu<I>>
  implements IHalulu<HaluluCommon<I, IH>>
{
  protected internal: IH;
  protected friends: HaluluCommon<I, IH>[] = [];

  constructor(internal: IH) {
    this.internal = internal;
  }

  public get name(): string {
    return this.internal.name;
  }

  public get age(): number {
    return this.internal.age;
  }

  public abstract get isCute(): boolean;

  public abstract setIsCute(isCute: boolean): void;

  public abstract sleep(): void;

  public abstract fetchFromServer(url: string): Promise<void>;

  public abstract saveToServer(url: string): Promise<void>;

  public sayHello(): string {
    return this.internal.sayHello();
  }

  public play(toy: string): string {
    return this.internal.play(toy);
  }

  public eat(food: string): void {
    this.internal.eat(food);
  }

  public playWith(friend: IHalulu<HaluluCommon<I, IH>>): string {
    const self = this as HaluluCommon<I, IH>;
    const other = friend as unknown as HaluluCommon<I, IH>;

    if (self === other) {
      return `${this.name} cannot play with itself!`;
    }

    if (!this.friends.includes(other)) {
      return `${this.name} is not friends with ${other.name}`;
    }

    return this.internal.playWith(other.internal);
  }

  public makeFriend(friend: HaluluCommon<I, IH>) {
    this.friends.push(friend);
    return this.internal.makeFriend(friend.internal);
  }

  public getFriends(): HaluluCommon<I, IH>[] {
    return this.friends;
  }

  public birthday(): void {
    this.internal.birthday();
  }

  public getAgeInDogYears(): number {
    return this.internal.getAgeInDogYears();
  }

  public setName(name: string): void {
    this.internal.setName(name);
  }

  public setAge(age: number): void {
    this.internal.setAge(age);
  }
}

export class HaluluWeb
  extends HaluluCommon<WebHaluluInterface>
  implements IHalulu
{
  setIsCute(_isCute: boolean): void {
    console.warn('isCute is not available on web');
  }

  public get isCute(): boolean {
    console.warn('isCute is not available on web');
    return false;
  }

  sleep(): void {
    console.warn('HaluluWeb cannot sleep');
  }

  fetchFromServer(url: string): Promise<void> {
    return this.internal.fetchFromServer(url);
  }

  saveToServer(url: string): Promise<void> {
    return this.internal.saveToServer(url);
  }
}

export class HaluluMobile
  extends HaluluCommon<MobileHaluluInterface>
  implements IHalulu
{
  public get isCute(): boolean {
    return this.internal.isCute;
  }

  setIsCute(isCute: boolean): void {
    this.internal.setIsCute(isCute);
  }

  sleep(): void {
    this.internal.sleep();
  }

  fetchFromServer(_url: string): Promise<void> {
    return Promise.reject(
      new Error('fetchFromServer is not available on mobile')
    );
  }

  saveToServer(_url: string): Promise<void> {
    return Promise.reject(new Error('saveToServer is not available on mobile'));
  }
}
