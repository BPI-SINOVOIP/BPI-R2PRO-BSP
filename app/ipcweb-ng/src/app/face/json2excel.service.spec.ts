import { TestBed } from '@angular/core/testing';

import { Json2excelService } from './json2excel.service';

describe('Json2excelService', () => {
  let service: Json2excelService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(Json2excelService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
