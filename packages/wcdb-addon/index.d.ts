export interface BusinessItemInput {
  title: string;
  content?: string;
}

export interface BusinessItem {
  id: number;
  title: string;
  content: string;
  updatedAt: number;
}

export interface ExtraInput {
  businessItemId: number;
  key: string;
  value?: string;
}

export interface Extra {
  id: number;
  businessItemId: number;
  key: string;
  value: string;
  updatedAt: number;
}

export interface BusinessItemApi {
  create(input: BusinessItemInput): Promise<BusinessItem>;
  get(id: number): Promise<BusinessItem | null>;
  list(): Promise<BusinessItem[]>;
  update(id: number, input: Partial<BusinessItemInput>): Promise<boolean>;
  remove(id: number): Promise<boolean>;
}

export interface ExtraApi {
  create(input: ExtraInput): Promise<Extra>;
  get(id: number): Promise<Extra | null>;
  list(): Promise<Extra[]>;
  listByBusinessItemId(businessItemId: number): Promise<Extra[]>;
  update(id: number, input: Partial<Omit<ExtraInput, "businessItemId">>): Promise<boolean>;
  remove(id: number): Promise<boolean>;
}

export class BusinessDb {
  businessItem: BusinessItemApi;
  extra: ExtraApi;

  constructor(dbPath: string);
  init(): Promise<boolean>;
  close(): Promise<void>;
}
