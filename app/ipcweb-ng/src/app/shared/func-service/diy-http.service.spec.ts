import { TestBed } from '@angular/core/testing';

import { DiyHttpService } from './diy-http.service';

describe('DiyHttpService', () => {
  let service: DiyHttpService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(DiyHttpService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
